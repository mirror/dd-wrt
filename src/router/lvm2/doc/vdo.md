# VDO - Compression and deduplication.

Currently device stacking looks like this:

    Physical x [multipath] x [partition] x [mdadm] x [LUKS] x [LVS] x [LUKS] x [FS|Database|...]

Adding VDO:

    Physical x [multipath] x [partition] x [mdadm] x [LUKS] x [LVS] x [LUKS] x VDO x [LVS] x [FS|Database|...]

## Where VDO fits (and where it does not):

### Backing devices for VDO volumes:

1. Physical x [multipath] x [partition] x [mdadm],
2. LUKS over (1) - full disk encryption.
3. LVs (raids|mirror|stripe|linear) x [cache] over (1).
4. LUKS over (3) - especially when using raids.

Usual limitations apply:

- Never layer LUKS over another LUKS - it makes no sense.
- LUKS is better over the raids, than under.

Devices which are not best suitable as backing device:

- thin volumes - at the moment it is not possible to take snapshot of active VDO volume on top of thin volume.

### Using VDO as a PV:

1. under tdata
    - The best fit - it will deduplicate additional redundancies among all
      snapshots and will reduce the footprint.
    - Risks: Resize! dmevent will not be able to handle resizing of tpool ATM.
2. under corig
    - This is useful to keep the most frequently used data in cache
      uncompressed or without deduplication if that happens to be a bottleneck.
    - Cache may fit better under VDO device, depending on compressibility and
      amount of duplicates, as
        - compression will reduce amount of data, thus effectively increasing
          size of cache,
        - and deduplication may emphasize hotspots.
    - Performance testing of your particular workload is strongly recommended.
3. under (multiple) linear LVs - e.g. used for VMs.

### And where VDO does not fit:

- *never* use VDO under LUKS volumes
    - these are random data and do not compress nor deduplicate well,
- *never* use VDO under cmeta and tmeta LVs
    - these are random data and do not compress nor deduplicate well,
- under raids
    - raid{4,5,6} scrambles data, so they do not deduplicate well,
    - raid{1,4,5,6,10} also causes amount of data grow, so more (duplicit in
      case of raid{1,10}) work has to be done in order to find less duplicates.

### And where it could be useful:

- under snapshot CoW device - when there are multiple of those it could deduplicate

## Development

### Things to decide

- under integrity devices
    - VDO should work well for data blocks,
    - but hashes are mostly unique and not compressible - were it possible it
      would make sense to have separate imeta and idata volumes for integrity
      devices.

### Future Integration of VDO into LVM:

One issue is using both LUKS and RAID under VDO. We have two options:

- use mdadm x LUKS x VDO+LV
- use LV RAID x LUKS x VDO+LV

In both cases dmeventd will not be able to resize the volume at the moment.

Another issue is duality of VDO - it can be used as a top level LV (with a
filesystem on top) but it can be used as "pool" for multiple devices too.

This will be solved in similar way thin pools allow multiple volumes.

Also VDO, has two sizes - its physical size and virtual size - and when
overprovisioning, just like tpool, we face same problems - VDO can get full,
without exposing it to a FS. dmeventd monitoring will be needed.

Another possible RFE is to split data and metadata - keep data on HDD and metadata on SSD.

## Issues / Testing

- fstrim/discard pass down - does it work with VDO?
- VDO can run in synchronous vs. asynchronous mode:
    - synchronous for devices where write is safe after it is confirmed. Some
      devices are lying.
    - asynchronous for devices requiring flush.
- Multiple devices under VDO - need to find and expose common properties, or
  not allow grouping them together. (This is same for all volumes with more
  physical devices below.)
- pvmove changing characteristics of underlying device.
- autoactivation during boot?
    - Q: can we use VDO for RootFS? Dracut!

