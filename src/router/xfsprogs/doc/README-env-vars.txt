Several environment variables are used within the code, primarily for
for debugging purposes; they are documented here.  In most cases simply
setting the environment variable enables the behavior, but in some
cases the value assigned influences behavior as well, as noted below.

General
-------
LIBXFS_LEAK_CHECK            -- warn and exit(1) if zone-allocated memory
                                is leaked at exit.
xfs_fsr
-------
FSRXFSTEST                   -- enable -C nfrag in theory coalesces into
                                nfrag extents.  Doesn't work.
xfs_scrub
---------
Known debug tweaks (pass -d and set the environment variable):
XFS_SCRUB_FORCE_ERROR        -- pretend all metadata is corrupt
XFS_SCRUB_FORCE_REPAIR       -- repair all metadata even if it's ok
XFS_SCRUB_NO_KERNEL          -- pretend there is no kernel ioctl
XFS_SCRUB_NO_SCSI_VERIFY     -- disable SCSI VERIFY (if present)
XFS_SCRUB_PHASE              -- run only this scrub phase
XFS_SCRUB_THREADS            -- start exactly this number of threads

Available even in non-debug mode:
SERVICE_MODE                 -- compress all error codes to 1 for LSB
                                service action compliance
