#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <linux/unistd.h>
#include <linux/cpufreq.h>
#include <pmc_drv.h>

#if defined CONFIG_THERMAL
#include <4908_map_part.h>

// TEMPERATURE LIMITS
#define BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES                         5

#define BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS     15000
#define BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS       10000

// QUAD CPU is meant to apply to the 4908
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS  110000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_2_MILLICELSIUS  125000

// DUAL CPU is meant to apply to the 4906
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS  100000
#define BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_2_MILLICELSIUS  110000

#define BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS      5000

/*-------------------------*
 *   CPU Cooling Devices   *
 *-------------------------*/

/* CPU PROCESSORS COLD COMPENSATION DEVICE */
static int broadcom_cpu_cold_compensation_lastAnnouncement = 2;

int broadcom_cpu_cold_compensation_get_max_state (struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 2;
  return 0;
}

int broadcom_cpu_cold_compensation_get_cur_state (struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_cpu_cold_compensation_set_cur_state (struct thermal_cooling_device *dev, unsigned long state)
{
  switch (state)
  {
    case 0:
      if (broadcom_cpu_cold_compensation_lastAnnouncement != 0) {
        dev_crit(&dev->device,"Handling Cold\n");
      RecloseAVS (1);
        broadcom_cpu_cold_compensation_lastAnnouncement = 0;
      }
    case 1:
      break;
    case 2:
      if (broadcom_cpu_cold_compensation_lastAnnouncement != 2) {
        dev_crit(&dev->device,"Go normal\n");
      RecloseAVS (0);
        broadcom_cpu_cold_compensation_lastAnnouncement = 2;
      }
      break;
  }
  return 0;
}

struct thermal_cooling_device_ops broadcomCpuColdCompensationOps =
{
  .get_max_state = broadcom_cpu_cold_compensation_get_max_state,
  .get_cur_state = broadcom_cpu_cold_compensation_get_cur_state,
  .set_cur_state = broadcom_cpu_cold_compensation_set_cur_state,
};

/* CPU PROCESSORS COOLING DEVICE */
 
int broadcom_cpu_cooling_get_max_state (struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 3;
  return 0;
}

int broadcom_cpu_cooling_get_cur_state (struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_cpu_cooling_set_cur_state (struct thermal_cooling_device *dev, unsigned long state)
{
  switch (state)
  {
    case 0:
      {
        int cpuIndex = 1;
        for ( ; cpuIndex < num_present_cpus(); cpuIndex++) {
          if (!cpu_online(cpuIndex)) {
            dev_crit(&dev->device,"turn on CPU#%d\n", cpuIndex);
            cpu_up(cpuIndex);
          }
        }
      }
      break;
    case 1:
      break;
    case 2:
      if (cpu_online(3)) {
      cpu_down(3);
        dev_crit(&dev->device,"turn off CPU #3\n");
      }
      break;
    case 3:
      {
        int cpuIndex = 1;
        for ( ; cpuIndex < num_present_cpus(); cpuIndex++) {
          if (cpu_online(cpuIndex)) {
            dev_crit(&dev->device,"turn off CPU#%d\n", cpuIndex);
            cpu_down(cpuIndex);
          }
        }
      }
      break;
  }
  return 0;
}

struct thermal_cooling_device_ops broadcomCpuCoolingOps =
{
  .get_max_state = broadcom_cpu_cooling_get_max_state,
  .get_cur_state = broadcom_cpu_cooling_get_cur_state,
  .set_cur_state = broadcom_cpu_cooling_set_cur_state,
};

/* CPU FREQUENCY COOLING DEVICE */

static int broadcom_cpufreq_cooling_lastAnnouncement = 0;

int broadcom_cpufreq_cooling_get_max_state (struct thermal_cooling_device *dev, unsigned long *states)
{
  *states = 2;
  return 0;
}

int broadcom_cpufreq_cooling_get_cur_state (struct thermal_cooling_device *dev, unsigned long *currentState)
{
  *currentState = 0;
  return 0;
}

int broadcom_cpufreq_cooling_set_cur_state (struct thermal_cooling_device *dev, unsigned long state)
{
  switch (state)
  {
    case 0:
      if (broadcom_cpufreq_cooling_lastAnnouncement != 0) {
        dev_crit(&dev->device,"Go to max frequency\n");
      cpufreq_set_freq_max (1);
        broadcom_cpufreq_cooling_lastAnnouncement = 0;
      }
    case 1:
      break;
    case 2:
      if (broadcom_cpufreq_cooling_lastAnnouncement != 2) {
        dev_crit(&dev->device,"Go to low frequency\n");
      cpufreq_set_freq_max (2);
        broadcom_cpufreq_cooling_lastAnnouncement = 2;
      }
      break;
  }
  return 0;
}

struct thermal_cooling_device_ops broadcomCpuFreqCoolingOps =
{
  .get_max_state = broadcom_cpufreq_cooling_get_max_state,
  .get_cur_state = broadcom_cpufreq_cooling_get_cur_state,
  .set_cur_state = broadcom_cpufreq_cooling_set_cur_state,
};
/*------------------------*
 * End CPU Cooling Device *
 *------------------------*/


/*--------------------*
 * Thermal Sensor     *
 *--------------------*/
 
struct volatile_thermal {
	uint32 __iomem volatile *tempSensor;
};

static int tripTemperatures[BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES];

int broadcomTempDrv_get_temp (struct thermal_zone_device *thermDev, int *tempMillicelsius)
{
  struct volatile_thermal *tempSensor = (struct volatile_thermal * ) thermDev->devdata;
  int regVal = *tempSensor->tempSensor;
  regVal &= 0x000003ff; 
  *tempMillicelsius = (4133500 - regVal * 4906) / 10;
  dev_dbg(&thermDev->device, "Temperature in Celsius              %d.%03d\n", *tempMillicelsius/1000,  *tempMillicelsius % 1000);
  return 0;  
}

int broadcomTempDrv_get_trip_type(struct thermal_zone_device *thermalZoneDev, int trip, enum thermal_trip_type *type)
{
   switch (trip) {
     case 0:
     case 1:
     case 2:
       *type = THERMAL_TRIP_ACTIVE;
       break;
     default:
      *type = THERMAL_TRIP_ACTIVE;
   }
   
   return 0;
}

int broadcomTempDrv_set_trip_temp (struct thermal_zone_device *thermalZoneDev, int trip, int tempMillicelsius) 
{
  if ((trip >= 0) && (trip < BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES))
  {
    tripTemperatures[trip] = tempMillicelsius;
  }
  return 0;
}


int broadcomTempDrv_get_trip_temp (struct thermal_zone_device *thermalZoneDev, int trip, int *tempMillicelsius)
{
   if ((trip >= 0) && (trip < BROADCOM_THERMAL_NUM_TRIP_TEMPERATURES))
   {
     *tempMillicelsius = tripTemperatures[trip];
     return 0;
   }

   return -1;
}

int broadcomTempDrv_get_trip_hyst (struct thermal_zone_device *thermalZoneDev, int trip, int *tempMillicelsius)
{
   switch (trip) {
     case 0:
     case 1:
     case 2:
     default:
      *tempMillicelsius = 0;
   }
   
   return 0;
}

int broadcomTempDrv_notify (struct thermal_zone_device *thermalZoneDev, int trip, enum thermal_trip_type type)
{
  return 0;
}
  
struct thermal_zone_device_ops thermalOps =
{
  .get_temp = broadcomTempDrv_get_temp,
  .set_trip_temp = broadcomTempDrv_set_trip_temp,
  .get_trip_type = broadcomTempDrv_get_trip_type,
  .get_trip_temp = broadcomTempDrv_get_trip_temp,
  .get_trip_hyst = broadcomTempDrv_get_trip_hyst,
  .notify = broadcomTempDrv_notify,
};

// the activate and register function
int broadcomTempDrv_init (struct platform_device *platDev)
{
  struct thermal_zone_device *thermalZoneDev;

  /* structs allocated by devm_ are automatically freed when device exits */
  struct volatile_thermal *tempSensor = NULL;
  struct thermal_zone_params *zoneParams = NULL;

  struct thermal_cooling_device *cpuCoolCompDev = thermal_cooling_device_register("passive", platDev, &broadcomCpuColdCompensationOps);
  struct thermal_cooling_device *cpuCoolDev = thermal_cooling_device_register("passive", platDev, &broadcomCpuCoolingOps);
  struct thermal_cooling_device *cpuFreqDev = NULL;

  int numCpus = num_present_cpus();

  dev_crit(&platDev->dev, "init (CPU count %d %d %d %d)\n", 
    num_present_cpus(),
    num_online_cpus(),
    num_possible_cpus(),
    num_active_cpus());

  if (numCpus == 2)
  {
    broadcomTempDrv_set_trip_temp (NULL, 0, BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 1, BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS + BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 2, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS - BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 3, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_1_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 4, BROADCOM_THERMAL_HIGH_TEMPERATURE_DUAL_CPU_COMPENSATION_2_MILLICELSIUS);
  }
  else if (numCpus == 4)
  {
    broadcomTempDrv_set_trip_temp (NULL, 0, BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 1, BROADCOM_THERMAL_LOW_TEMPERATURE_HYSTERESIS_MILLICELSIUS + BROADCOM_THERMAL_LOW_TEMPERATURE_COMPENSATION_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 2, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS - BROADCOM_THERMAL_HIGH_TEMPERATURE_HYSTERESIS_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 3, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_1_MILLICELSIUS);
    broadcomTempDrv_set_trip_temp (NULL, 4, BROADCOM_THERMAL_HIGH_TEMPERATURE_QUAD_CPU_COMPENSATION_2_MILLICELSIUS);
  }
  else 
  {
    dev_err(&platDev->dev, "Can't handle %d CPUs\n", numCpus);
    return -1;
  }

  if ((cpuCoolDev == NULL) || (cpuCoolCompDev == NULL))
  {
    dev_err(&platDev->dev, "Can't create one of the cooling devices\n");
    return -1;
  }

  if (numCpus == 2) 
  {
    cpuFreqDev = thermal_cooling_device_register("passive", platDev, &broadcomCpuFreqCoolingOps);

    if (cpuFreqDev == NULL)
    {
      dev_err(&platDev->dev, "Can't create frequency cooling device\n");
      return -1;
    }
  }  

  zoneParams = devm_kzalloc(&platDev->dev, sizeof(struct thermal_zone_params), GFP_KERNEL);

  strcpy (zoneParams->governor_name, "step_wise");
  zoneParams->no_hwmon = true;
  zoneParams->num_tbps = 0;

  tempSensor = devm_kzalloc(&platDev->dev, sizeof(struct volatile_thermal), GFP_KERNEL);
  if (!tempSensor)
    return -ENOMEM;
  
  tempSensor->tempSensor = &BIUCTRL->therm_throttle_temp;

  thermalZoneDev = thermal_zone_device_register(
    "broadcomThermalDrv", /* name */
    5,                    /* trips */
    0,                    /* mask */
    (void *)tempSensor,   /* device data */ 
    &thermalOps,
    zoneParams,           /* thermal zone params */
    1000,                 /* passive delay */
    1000                  /* polling delay */
    );

  if (IS_ERR(thermalZoneDev)) {
    dev_err(&platDev->dev, "Failed to register thermal zone device\n");
    return PTR_ERR(thermalZoneDev);
  }

  platform_set_drvdata(platDev, thermalZoneDev);


  // Configure cold compensation for all known boards
  thermal_zone_bind_cooling_device(thermalZoneDev, 0 /*trip*/, 
                                   cpuCoolCompDev, 
                                   1, 1, THERMAL_WEIGHT_DEFAULT); // Go to high gain
  thermal_zone_bind_cooling_device(thermalZoneDev, 1 /*trip*/, 
                                   cpuCoolCompDev, 
                                   2, 2, THERMAL_WEIGHT_DEFAULT); // Go to normal gain

  if (numCpus == 4)
  {
    // configure heat compensation for 4908 (and other 4 CPU units)
    thermal_zone_bind_cooling_device(thermalZoneDev, 2 /*trip*/, 
                                     cpuCoolDev, 
                                     1, 1, THERMAL_WEIGHT_DEFAULT); // do nothing state
    thermal_zone_bind_cooling_device(thermalZoneDev, 3 /*trip*/, 
                                     cpuCoolDev, 
                                     2, 2, THERMAL_WEIGHT_DEFAULT); // Shut down 1 CPU
    thermal_zone_bind_cooling_device(thermalZoneDev, 4 /*trip*/, 
                                     cpuCoolDev, 
                                     3, 3, THERMAL_WEIGHT_DEFAULT); // Shut down all but 1 CPU
  }
  else
  {
    // configure heat compensation for 4906 (and other 2 CPU units)
    thermal_zone_bind_cooling_device(thermalZoneDev, 2 /*trip*/, 
                                     cpuCoolDev, 
                                     1, 1, THERMAL_WEIGHT_DEFAULT); // do nothing
    thermal_zone_bind_cooling_device(thermalZoneDev, 3 /*trip*/, 
                                     cpuCoolDev, 
                                     3, 3, THERMAL_WEIGHT_DEFAULT); // Shut down all but 1 CPU
                                     
    thermal_zone_bind_cooling_device(thermalZoneDev, 2 /*trip*/, 
                                     cpuFreqDev, 
                                     1, 1, THERMAL_WEIGHT_DEFAULT); // do nothing
    thermal_zone_bind_cooling_device(thermalZoneDev, 4 /*trip*/, 
                                     cpuFreqDev, 
                                     2, 2, THERMAL_WEIGHT_DEFAULT); // Turn down CPU freq on last CPU
  }

  return 0;
}

int broadcomTempDrv_remove (struct platform_device *platDev)
{
  struct thermal_zone_device *thermalZoneDev = platform_get_drvdata(platDev);

  thermal_zone_device_unregister(thermalZoneDev);

  return 0;
}

static const struct of_device_id broadcom_thermal_id_table[] = {
	{ .compatible = "brcm,therm" },
	{}
};

struct platform_driver broadcomThermalDriver = {
  .probe = broadcomTempDrv_init,  
  .remove = broadcomTempDrv_remove,
  .driver = {
    .name = "broadcomThermalDrv",
    .of_match_table = broadcom_thermal_id_table  },
};

module_platform_driver(broadcomThermalDriver);

MODULE_DESCRIPTION("Broadcom thermal driver");
MODULE_LICENSE("GPL");

#endif //CONFIG_THERMAL
