# MMRC Module API

## Design Philosophy

The MMRC Module API was designed with the following design philosophies in mind:

* Minimising single point in time knowledge

At any time, The MMRC module only ever needs to know the information
about the single STA it is working with. At this point it should be able
to read, configure and update the appropriate tables then forget about them
entirely.

* Minimise systems it needs to interact with

By leaning on one external system, in this case the Morse Micro driver, the MMRC
module will generate, at initialisation time, enough information to process
packets appropriately for the lifecycle of the STA. Additionally any environment
the Morse Micro driver supports will support the MMRC module.

## API

### Life Cycle
The life cycle of the MMRC module works on a per STA basis where state is
allocated for each STA and then destroyed. There are three ways to control the
life cycle of the module:

1) Statically allocate an mmrc_table_t and request that the module initialises
it.
2) Dynamically allocate some memory of sufficient size, according to the size
required by mmrc_memory_required_for_caps and request that the module
initialises it. The memory must be freed when finished with.
3) Use the _create and _destroy functions which do the same as 2), but performs
the allocation and initialisation in a single step. Assumes that an OSAL exists
to perform memory allocation.

The capabilities provide the details on what features are supported by a
particular station, such as spatial streams, channel widths, etc.

```c
extern void
mmrc_sta_init(struct mmrc_table *tb, mmrc_sta_capabilities *caps);

extern size_t
mmrc_memory_required_for_caps(struct mmrc_sta_capabilities *caps);

extern mmrc_table_t *
mmrc_sta_create(struct mmrc_sta_capabilities *caps);

extern void
mmrc_sta_destroy(struct mmrc_table *tb);
```

### Update loop

The MMRC module requires periodic notification to ensure that internal state is
kept up to date. An update function is provided that we suggest is run
according to the MMRC_UPDATE_FREQUENCY_MS. The update function is passed a
delta_time_ms which represents the delta time from the last update and can be
used in instances where the update time is different to MMRC_UPDATE_FREQUENCY_MS
or there is some unexpected variation in the time since the last update.

```c
extern void
mmrc_update(struct mmrc_table *tb, uint32_t delta_time_ms);
```

### Packet rate selection and feedback

The MMRC module will provide, on request for a packet of size_t, a rate table
consisting of four mmrc_rate. Each mmrc_rate contains the relevant information
for the rate and the number of attempts.

In response to this, the MMRC modules expects feedback with regard to the
amount of retries that were used in the rate table. This feedback comes in two
forms, one for a chain that is used for a set of aggregated frames and one for
any other individual frames. The difference being that the aggregated feedback
needs to feedback how many frames were successfully sent and how many failed to
send.

```c
extern void
mmrc_get_rates(struct mmrc_table *tb, struct mmrc_rate_table *out, size_t size);
```

```c
extern void
mmrc_feedback(struct mmrc_table *tb,
              struct mmrc_rate_table *chain,
              int32_t retry_count);
```

```c
extern void
mmrc_feedback_agg(struct mmrc_table *tb,
                  struct mmrc_rate_table *rates,
                  int32_t retry_count,
                  uint32_t success,
                  uint32_t failure);
```

## Example Usage

### Configure a newly associated STA

```c
/* Create a new table for a newly Associated STA, you must keep this struct around for the life
 * of the STA */
struct mmrc_table table;

/* Set up the capabilities for this new STA, after mmrc_sta_init is called, this is no longer
 * needed */
struct mmrc_sta_capabilities caps;

/* Configure STA for support up to 8MHZ */
caps.bw = MMRC_MASK(MMRC_BW_2MHZ) |
          MMRC_MASK(MMRC_BW_4MHZ) |
          MMRC_MASK(MMRC_BW_8MHZ);
/* Configure STA for support for 3 Spatial Streams */
caps.spatial_streams = MMRC_MASK(MMRC_SPATIAL_STREAM_1) |
                       MMRC_MASK(MMRC_SPATIAL_STREAM_2) |
                       MMRC_MASK(MMRC_SPATIAL_STREAM_3);
/* Configure STA for support for MCS1 -> MCS9 */
caps.rates = MMRC_MASK(MMRC_MCS0) | MMRC_MASK(MMRC_MCS1) |
             MMRC_MASK(MMRC_MCS2) | MMRC_MASK(MMRC_MCS3) |
             MMRC_MASK(MMRC_MCS4) | MMRC_MASK(MMRC_MCS5) |
             MMRC_MASK(MMRC_MCS6) | MMRC_MASK(MMRC_MCS7) |
             MMRC_MASK(MMRC_MCS8) | MMRC_MASK(MMRC_MCS9);
/* Configure STA for short and long guard */
caps.guard = MMRC_MASK(MMRC_GUARD_LONG) | MMRC_MASK(MMRC_GUARD_SHORT);

/* Initialise the STA in MMRC */
mmrc_sta_init(&table, &caps);
```

### Update MMRC

Every update period, which we suggest defaults to MMRC_UPDATE_FREQUENCY_MS this should be done

```c
/* Request MMRC updates statistics for rate generation */
mmrc_update(&table, MMRC_UPDATE_FREQUENCY_MS);
```

### Request rates and provide feedback

The process for requesting rates does not change between aggregated and non aggregated packets
```c
/* Prepare an empty rate table to be filled by MMRC */
struct mmrc_rate_table out;

/* Get size of frame/frames*/
size_t size = foo_get_size();

/* Request a new rate table for aggregated/non aggregated frames */
mmrc_get_rates(&table, &out, size);

/* Send frames */
foo();

/* Give MMRC feedback via aggregation or non aggregation method */
mmrc_feedback_agg(table, out, 14, 10, 0);
mmrc_feedback(table, out, 14);
```


## MMRC table generation

A table is generated from the STA capabilities struct that is initiailised on
mmrc_init(). With this information a table can be ordered that we can use to
infer the specific features without explicitly storing them.

In this example below we can see what the fields in a table might look like
based on there ordering. In this example there are 2 bandwidths supported,
2 and 4 MHz respectively, 1 spatial stream, and an uncertain number of MCS.

|Order Priority 1|Order Priority 2|...|
|-|-|-|
| MCS0 | 1SS | BW_2 |
| MCS0 | 1SS | BW_4 |
| MCS1 | 1SS | BW_2 |
| MCS1 | 1SS | BW_4 |
| MCS2 | 1SS | BW_2 |
| MCS2 | 1SS | BW_4 |
| ... | | |

In reality the table actually looks as follows...

| prob | sent | sent_success | throughput |
|-|-|-|-|
|0.59|0|0|2|
|0.89|0|0|4|
|...||||