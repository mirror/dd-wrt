/*
 * Copyright 2024 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>
#include <linux/utsname.h>
#include <linux/devcoredump.h>
#include <linux/elf.h>

#include "morse.h"
#include "debug.h"
#include "coredump.h"
#include "firmware.h"
#include "bus.h"
#include "ps.h"

#define MORSE_CHIP_HALT_TRGR_SET(_m)      MORSE_REG_TRGR1_SET(_m)
#define MORSE_CHIP_HALT_IRQ_BIT           BIT(30)
#define MORSE_CHIP_HALT_DELAY_MS          10

#define MORSE_COREDUMP_DBG(_m, _f, _a...)   morse_dbg(FEATURE_ID_COREDUMP, _m, _f, ##_a)
#define MORSE_COREDUMP_INFO(_m, _f, _a...)  morse_info(FEATURE_ID_COREDUMP, _m, _f, ##_a)
#define MORSE_COREDUMP_WARN(_m, _f, _a...)  morse_warn(FEATURE_ID_COREDUMP, _m, _f, ##_a)
#define MORSE_COREDUMP_ERR(_m, _f, _a...)   morse_err(FEATURE_ID_COREDUMP, _m, _f, ##_a)

/* Bitfield positions of optional data to include in the coredump */
enum coredump_optional_flags {
	/* Include memory regions extracted from the chip */
	COREDUMP_OPTIONAL_FLAGS_CHIP_MEMORY = 0,
};

/* Enable/Disable Coredump */
static bool enable_coredump __read_mostly = true;
module_param(enable_coredump, bool, 0644);
MODULE_PARM_DESC(enable_coredump, "Enable coredump creation on chip failure");

/* Method for coredump creation */
static enum morse_coredump_method coredump_method __read_mostly = COREDUMP_METHOD_BUS;
module_param(coredump_method, uint, 0644);
MODULE_PARM_DESC(coredump_method, "Method of coredump creation");

/* Data to include in the coredump. Only applicable to coredumps generated with
 * COREDUMP_METHOD_BUS.
 */
static unsigned long coredump_include __read_mostly = BIT(COREDUMP_OPTIONAL_FLAGS_CHIP_MEMORY);
module_param(coredump_include, ulong, 0644);
MODULE_PARM_DESC(coredump_include, "Bitfield describing optional data to include in the coredump");

struct morse_elf_note {
	struct list_head list;
	enum morse_coredump_note_type type;
	size_t namesz;
	size_t datasz;
	/* Contains the name + data */
	u8 variable[];
};

static int read_memory_region(struct morse *mors,
						const struct morse_coredump_mem_region *region,
						void *copy_to)
{
	int ret;
	void *buffer;

	if (WARN_ON(ROUND_BYTES_TO_WORD(region->len) > INT_MAX)) {
		MORSE_COREDUMP_ERR(mors, "%s: invalid length for region 0x%08x:%u",
				   __func__, region->start, region->len);
		return -EINVAL;
	}

	buffer = kzalloc(ROUND_BYTES_TO_WORD(region->len), GFP_KERNEL);
	if (!buffer) {
		MORSE_COREDUMP_ERR(mors, "%s: failed to alloc buffer for 0x%08x:%u",
						   __func__,
						   region->start,
						   region->len);
		return -ENOMEM;
	}

	/* Note: Data must be copied to an intermediate buffer as BUS transactions
	 *       cannot write directly into virtual memory.
	 */
	if (region->len == sizeof(u32))
		ret = morse_reg32_read(mors, region->start, buffer);
	else
		ret = morse_dm_read(mors, region->start, buffer,
				    (int)ROUND_BYTES_TO_WORD(region->len));

	if (!ret)
		memcpy(copy_to, buffer, region->len);
	else
		MORSE_COREDUMP_ERR(mors, "%s: failed to read memory 0x%08x:%u",
						   __func__,
						   region->start,
						   region->len);

	kfree(buffer);
	return ret;
}

static int meta_append(struct list_head *notes,
					const char *name,
					const void *data,
					size_t len,
					enum morse_coredump_note_type type)
{
	struct morse_elf_note *note;
	u8 *insert_at;
	size_t datasz = len;
	/* round up to ensure alignment between name and note contents */
	size_t namesz = ROUND_BYTES_TO_WORD(strlen(name) + 1);

	note = kzalloc(struct_size(note, variable, namesz + datasz), GFP_KERNEL);
	if (!note)
		return -ENOMEM;

	note->type = type;
	note->namesz = namesz;
	note->datasz = datasz;

	insert_at = note->variable;
	memcpy(insert_at, name, strlen(name));
	insert_at += namesz;
	memcpy(insert_at, data, len);
	list_add_tail(&note->list, notes);
	return 0;
}

static int meta_append_u32(struct list_head *notes, const char *name, const void *data)
{
	return meta_append(notes,
		name, data, sizeof(u32), MORSE_COREDUMP_NOTE_TYPE_U32);
}

static int meta_append_u64(struct list_head *notes, const char *name, const void *data)
{
	return meta_append(notes,
		name, data, sizeof(u64), MORSE_COREDUMP_NOTE_TYPE_U64);
}

static int meta_append_str(struct list_head *notes, const char *name, const void *data)
{
	return meta_append(notes,
		name, data, strlen(data) + 1, MORSE_COREDUMP_NOTE_TYPE_STR);
}

static int meta_append_bin(struct list_head *notes, const char *name, const void *data, size_t len)
{
	return meta_append(notes,
		name, data, len, MORSE_COREDUMP_NOTE_TYPE_BIN);
}

static size_t elf_size_of_all_memory_regions(struct morse *mors, size_t *count)
{
	size_t size = 0;
	const struct morse_coredump_mem_region *region;
	const struct morse_coredump_data *crash = &mors->coredump.crash;

	lockdep_assert_held(&mors->coredump.lock);

	list_for_each_entry(region, &crash->memory.regions, list) {
		size += sizeof(struct elf32_phdr);
		size += region->len;
		*count += 1;
	}

	return size;
}

static size_t elf_size_of_all_notes(struct morse *mors,
								const struct list_head *notes,
								size_t *count)
{
	size_t size = 0;
	struct morse_elf_note *note;

	list_for_each_entry(note, notes, list) {
		size += sizeof(struct elf32_phdr);

		/* Need to ensure alignment within data section */
		size += ROUND_BYTES_TO_WORD(sizeof(struct elf32_note) +
			note->namesz + note->datasz);

		*count += 1;
	}

	return size;
}

static void elf_copy_notes(struct morse *mors,
						const struct list_head *notes,
						struct elf32_hdr *ehdr,
						struct elf32_phdr **phdr,
						size_t *offset)
{
	struct morse_elf_note *note;

	lockdep_assert_held(&mors->coredump.lock);

	list_for_each_entry(note, notes, list) {
		u8 *insert_at = (u8 *)ehdr + *offset;
		struct elf32_note *enote = (struct elf32_note *)insert_at;

		MORSE_COREDUMP_DBG(mors, "%s: copying note %s", __func__, note->variable);

		/* init program header */
		(*phdr)->p_type = PT_NOTE;
		(*phdr)->p_offset = *offset;
		(*phdr)->p_filesz =
			ROUND_BYTES_TO_WORD(sizeof(*enote) + note->namesz + note->datasz);
		(*phdr)->p_memsz = (*phdr)->p_filesz;

		/* init note header */
		enote->n_type = (__force u32)cpu_to_le32(note->type);
		enote->n_namesz = (__force u32)cpu_to_le32(note->namesz);
		enote->n_descsz = (__force u32)cpu_to_le32(note->datasz);

		/* copy in note name + data */
		insert_at += sizeof(*enote);
		memcpy(insert_at, note->variable, note->namesz + note->datasz);

		/* advance data offset pointer */
		*offset += (*phdr)->p_filesz;
		*phdr += 1;
	}
}

static void get_stop_info(struct morse *mors)
{
	const struct morse_coredump_mem_region *region;
	const struct morse_coredump_data *crash = &mors->coredump.crash;

	lockdep_assert_held(&mors->coredump.lock);
	list_for_each_entry(region, &crash->memory.regions, list) {
		struct stop_info {
			__le32 hart;
			__le32 line;
			char info[];
		} __packed * info;

		if (region->type != MORSE_MEM_REGION_TYPE_ASSERT_INFO)
			continue;

		MORSE_COREDUMP_DBG(mors, "%s: looking in region 0x%08x:%d",
			__func__, region->start, region->len);

		if (region->len < (sizeof(*info) + 1)) {
			MORSE_COREDUMP_WARN(mors, "%s: size of info region is unexpected",
					    __func__);
			continue;
		}

		info = kzalloc(ROUND_BYTES_TO_WORD(region->len + 1), GFP_KERNEL);
		if (!info) {
			MORSE_COREDUMP_ERR(mors, "%s: failed to allocate %d bytes",
					   __func__, region->len);
			continue;
		}

		if (read_memory_region(mors, region, info) == 0) {
			uint hart = le32_to_cpu(info->hart);
			uint line = le32_to_cpu(info->line);

			if (strlen(info->info) > 0 || line > 0) {
				mors->coredump.crash.information = kasprintf(GFP_KERNEL,
									     "%s:%d (hart:%d)",
									     info->info, line,
									     hart);
			}
		}

		kfree(info);
		if (mors->coredump.crash.information) {
			MORSE_COREDUMP_ERR(mors, "stop at %s\n",
					   mors->coredump.crash.information);
			break;
		}
	}
}

static void elf_copy_memory_regions(struct morse *mors,
								struct elf32_hdr *ehdr,
								struct elf32_phdr **phdr,
								size_t *offset)
{
	const struct morse_coredump_mem_region *region;
	const struct morse_coredump_data *crash = &mors->coredump.crash;

	lockdep_assert_held(&mors->coredump.lock);

	list_for_each_entry(region, &crash->memory.regions, list) {
		u8 *insert_at = (u8 *)ehdr + *offset;

		if (region->type != MORSE_MEM_REGION_TYPE_GENERAL)
			continue;

		MORSE_COREDUMP_DBG(mors, "%s: copying region 0x%08x:%d",
			__func__, region->start, region->len);

		(*phdr)->p_type = PT_LOAD;
		(*phdr)->p_offset =  *offset;
		(*phdr)->p_vaddr = region->start;
		(*phdr)->p_paddr = region->start;
		(*phdr)->p_filesz = region->len;
		(*phdr)->p_memsz = region->len;
		(*phdr)->p_flags = PF_R | PF_W | PF_X;
		(*phdr)->p_align = 0;

		if (read_memory_region(mors, region, insert_at)) {
			/* Failed to read the memory region */
			(*phdr)->p_filesz = 0;
			(*phdr)->p_memsz = 0;
		}

		*offset += (*phdr)->p_filesz;
		*phdr += 1;
	}
}

static void elf_init_header(struct morse *mors, struct elf32_hdr *ehdr, size_t phnum)
{
	memcpy(ehdr->e_ident, ELFMAG, SELFMAG);
	ehdr->e_ident[EI_CLASS] = ELFCLASS32;
	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_ident[EI_VERSION] = EV_CURRENT;
	ehdr->e_ident[EI_OSABI] = ELFOSABI_NONE;
	ehdr->e_type = ET_CORE;
	ehdr->e_machine = EM_RISCV;
	ehdr->e_version = EV_CURRENT;
	ehdr->e_ehsize = sizeof(*ehdr);
	ehdr->e_phoff = ehdr->e_ehsize;
	ehdr->e_phentsize = sizeof(struct elf32_phdr);
	ehdr->e_phnum = phnum;
}

static void add_coredump_meta(const struct morse *mors, struct list_head *notes)
{
	const struct morse_coredump_data *crash = &mors->coredump.crash;
	u32 cd_file_version = (__force u32)cpu_to_le32(MORSE_COREDUMP_FILE_VERSION);
	u32 rel_major = mors->sw_ver.major;
	u32 rel_minor = mors->sw_ver.minor;
	u32 rel_patch = mors->sw_ver.patch;

	lockdep_assert_held(&mors->coredump.lock);

	meta_append_u32(notes, "morse.cd-version", &cd_file_version);
	meta_append_u32(notes, "morse.cd-reason", &crash->reason);
	meta_append_u32(notes, "morse.chip-id", &mors->chip_id);
	meta_append_u32(notes, "morse.bus", &mors->bus_type);
	meta_append_u64(notes, "morse.ts-sec", &crash->timestamp.tv_sec);
	meta_append_u64(notes, "morse.ts-nsec", &crash->timestamp.tv_nsec);
	meta_append_u32(notes, "morse.rel-major", &rel_major);
	meta_append_u32(notes, "morse.rel-minor", &rel_minor);
	meta_append_u32(notes, "morse.rel-patch", &rel_patch);
	meta_append_u32(notes, "morse.fw-flags", &mors->firmware_flags);
	if (mors->coredump.fw_binary_str)
		meta_append_str(notes, "morse.fw-binary", mors->coredump.fw_binary_str);
	if (mors->coredump.fw_ver_str)
		meta_append_str(notes, "morse.fw-version", mors->coredump.fw_ver_str);
	meta_append_str(notes, "morse.bcf-serial", mors->board_serial);
	meta_append_str(notes, "morse.kernel-version", init_utsname()->release);
	meta_append_str(notes, "morse.country", mors->country);
	meta_append_bin(notes, "morse.mac-addr", mors->macaddr, ETH_ALEN);
	if (mors->coredump.crash.information)
		meta_append_str(notes, "morse.stop-info", mors->coredump.crash.information);
}

static int coredump_build(struct morse *mors, void **cd, size_t *cd_size)
{
	int ret;
	size_t file_size = 0;
	struct elf32_hdr *ehdr;
	struct elf32_phdr *phdr;
	size_t offset;
	size_t phnum = 0;
	struct list_head notes;
	struct morse_elf_note *note;
	struct morse_elf_note *tmp;

	lockdep_assert_held(&mors->coredump.lock);

	/* Claim/release bus for the entire bus access operation */
	morse_claim_bus(mors);

	get_stop_info(mors);
	INIT_LIST_HEAD(&notes);
	add_coredump_meta(mors, &notes);

	file_size = sizeof(*ehdr);
	file_size += elf_size_of_all_memory_regions(mors, &phnum);
	file_size += elf_size_of_all_notes(mors, &notes, &phnum);

	*cd = vzalloc(file_size);
	if (!(*cd)) {
		*cd = NULL;
		*cd_size = 0;
		ret = -ENOMEM;
		goto exit;
	}
	*cd_size = file_size;

	/* Fill the elf header */
	ehdr = (struct elf32_hdr *)*cd;
	elf_init_header(mors, ehdr, phnum);

	/* Calculate the start of the program headers, and the .data offset */
	phdr = (struct elf32_phdr *)((u8 *)ehdr + ehdr->e_phoff);
	offset = sizeof(*ehdr) + (sizeof(*phdr) * ehdr->e_phnum);

	/* Insert memory regions */
	elf_copy_memory_regions(mors, ehdr, &phdr, &offset);

	morse_release_bus(mors);

	/* Insert notes */
	elf_copy_notes(mors, &notes, ehdr, &phdr, &offset);

	MORSE_COREDUMP_DBG(mors, "%s: elf size: %zu, n program headers: %zu",
		__func__,
		file_size,
		phnum);

	ret = 0;
exit:
	/* Delete the notes */
	list_for_each_entry_safe(note, tmp, &notes, list) {
		list_del(&note->list);
		kfree(note);
	}
	return ret;
}

static int coredump_submit(struct morse *mors)
{
	int ret;
	void *coredump;
	size_t coredump_size;

	lockdep_assert_held(&mors->coredump.lock);

	ret = coredump_build(mors, &coredump, &coredump_size);
	if (ret) {
		MORSE_COREDUMP_ERR(mors, "%s: failed to produce crash data\n", __func__);
		goto exit;
	}

	/* coredump is consumed and free'd by the devcoredumpv API */
	dev_coredumpv(mors->dev, coredump, coredump_size, GFP_KERNEL);
	ret = 0;

exit:
	/* reset reason state */
	mors->coredump.crash.reason = MORSE_COREDUMP_REASON_UNKNOWN;
	return ret;
}

static int bus_coredump(struct morse *mors)
{
	int ret = 0;

	mutex_lock(&mors->coredump.lock);
	coredump_submit(mors);
	mutex_unlock(&mors->coredump.lock);

	return ret;
}

static int userspace_coredump(struct morse *mors)
{
	int ret;
	static const char *const envp[] = { "HOME=/", NULL };
	static const char *const argv[] = {
		"/bin/sh", "-c", "/usr/sbin/morse-core-dump.sh -d", NULL };

#if KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE
	ret = call_usermodehelper(argv[0], (char **)argv, (char **)envp, UMH_WAIT_PROC);
#else
	ret = call_usermodehelper((char *)argv[0], (char **)argv, (char **)envp, UMH_WAIT_PROC);
#endif

	return ret;
}

int morse_coredump(struct morse *mors)
{
	int ret = 0;
	bool use_userspace;
	enum morse_coredump_method method = coredump_method;

	lockdep_assert_held(&mors->lock);

	/* The watchdog should be paused, and power-save disabled */
	if (mors->watchdog.consumers > 0)
		MORSE_WARN_ON(FEATURE_ID_COREDUMP, !mors->watchdog.paused);
	MORSE_WARN_ON(FEATURE_ID_COREDUMP, mors->ps.suspended);

	/* Use userspace coredump method if;
	 * - The driver has been loaded with this specific coredump method
	 * - OR
	 * - The firmware binary does not describe any regions to dump, and
	 *   chip memory has not been explicitly omitted from the include modparam
	 */
	use_userspace = (method == COREDUMP_METHOD_USERSPACE_SCRIPT) ||
		(list_empty(&mors->coredump.crash.memory.regions) &&
		 test_bit(COREDUMP_OPTIONAL_FLAGS_CHIP_MEMORY, &coredump_include));

	if (use_userspace)
		method = COREDUMP_METHOD_USERSPACE_SCRIPT;

	/* Trigger a crash on-chip to force it to stop and save state. Will have
	 * no affect if chip has already crashed
	 */
	if (mors->firmware_flags & MORSE_FW_FLAGS_SUPPORT_CHIP_HALT_IRQ) {
		morse_claim_bus(mors);
		ret = morse_reg32_write(mors, MORSE_CHIP_HALT_TRGR_SET(mors),
								MORSE_CHIP_HALT_IRQ_BIT);
		morse_release_bus(mors);

		if (ret) {
			MORSE_COREDUMP_WARN(mors,
				"%s: Failed to halt chip with IRQ (ret:%d)", __func__, ret);
		} else {
			MORSE_COREDUMP_DBG(mors, "%s: Halted chip with IRQ", __func__);
			mdelay(MORSE_CHIP_HALT_DELAY_MS);
		}
	}

	if (mors->cfg->pre_coredump_hook)
		ret = mors->cfg->pre_coredump_hook(mors, method);

	if (ret)
		goto exit;

	switch (method) {
	case COREDUMP_METHOD_USERSPACE_SCRIPT:
		ret = userspace_coredump(mors);
		break;
	case COREDUMP_METHOD_BUS:
		ret = bus_coredump(mors);
		break;
	default:
		ret = -1;
		MORSE_COREDUMP_ERR(mors,
			"%s: unknown coredump method: %d\n", __func__, method);
		break;
	}

	if (mors->cfg->post_coredump_hook)
		ret = mors->cfg->post_coredump_hook(mors, method);

exit:
	if (ret)
		MORSE_COREDUMP_ERR(mors, "%s: failed to coredump: %d\n", __func__, ret);

	return ret;
}

int morse_coredump_new(struct morse *mors, enum morse_coredump_reason reason)
{
	struct morse_coredump_data *data = &mors->coredump.crash;

	if (!enable_coredump)
		return -ENOTSUPP;

	lockdep_assert_held(&mors->lock);

	mutex_lock(&mors->coredump.lock);
	data->reason = reason;
	ktime_get_real_ts64(&data->timestamp);
	mutex_unlock(&mors->coredump.lock);

	return 0;
}

int morse_coredump_add_memory_region(struct morse *mors,
	const struct morse_fw_info_tlv_coredump_mem *region)
{
	struct morse_coredump_mem_region *item;

	/* Not allowed */
	if (!test_bit(COREDUMP_OPTIONAL_FLAGS_CHIP_MEMORY, &coredump_include))
		return 0;

	item = kzalloc(sizeof(*item), GFP_KERNEL);
	if (!item)
		return -ENOMEM;

	mutex_lock(&mors->coredump.lock);

	item->type = le32_to_cpu(region->region_type);
	item->start = le32_to_cpu(region->start);
	item->len = le32_to_cpu(region->len);
	list_add_tail(&item->list, &mors->coredump.crash.memory.regions);

	mutex_unlock(&mors->coredump.lock);

	return 0;
}

void morse_coredump_remove_memory_regions(struct morse *mors)
{
	struct morse_coredump_data *crash = &mors->coredump.crash;
	struct morse_coredump_mem_region *region, *tmp;

	mutex_lock(&mors->coredump.lock);

	list_for_each_entry_safe(region, tmp, &crash->memory.regions, list) {
		list_del(&region->list);
		kfree(region);
	}

	mutex_unlock(&mors->coredump.lock);
}

void morse_coredump_set_fw_binary_str(struct morse *mors, const char *str)
{
	mutex_lock(&mors->coredump.lock);

	kfree(mors->coredump.fw_binary_str);
	mors->coredump.fw_binary_str = NULL;

	if (str)
		mors->coredump.fw_binary_str = kstrdup_const(str, GFP_KERNEL);

	mutex_unlock(&mors->coredump.lock);
}

void morse_coredump_set_fw_version_str(struct morse *mors, const char *str)
{
	mutex_lock(&mors->coredump.lock);

	kfree(mors->coredump.fw_ver_str);
	mors->coredump.fw_ver_str = NULL;

	if (str)
		mors->coredump.fw_ver_str = kstrdup_const(str, GFP_KERNEL);

	mutex_unlock(&mors->coredump.lock);
}

static void coredump_clear_stop_info(struct morse *mors)
{
	mutex_lock(&mors->coredump.lock);

	kfree(mors->coredump.crash.information);
	mors->coredump.crash.information = NULL;

	mutex_unlock(&mors->coredump.lock);
}

void morse_coredump_destroy(struct morse *mors)
{
	morse_coredump_remove_memory_regions(mors);
	morse_coredump_set_fw_binary_str(mors, NULL);
	morse_coredump_set_fw_version_str(mors, NULL);
	coredump_clear_stop_info(mors);
}
