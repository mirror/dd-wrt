obj-$(CONFIG_DMA_SHARED_BUFFER) := dma-shared-buffer.o

dma-buf-objs-y := dma-buf.o dma-fence.o dma-fence-array.o reservation.o seqno-fence.o
dma-buf-objs-$(CONFIG_SYNC_FILE)		+= sync_file.o
dma-buf-objs-$(CONFIG_SW_SYNC)		+= sw_sync.o sync_debug.o

dma-shared-buffer-objs :=  $(dma-buf-objs-y)
