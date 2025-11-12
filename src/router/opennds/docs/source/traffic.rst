Data Quotas and Traffic Shaping
###############################

Data volume and Rate Quotas
***************************

openNDS (NDS) has built in *Data Volume* and *Data Rate* quota support.

Data volume and data rate quotas can be set globally in the config file.

The global values can be overridden on a client by client basis as required.

Global Data Volume Quota
------------------------
If a client exceeds the global data volume quota, or the individual client quota, that client will be deauthenticated or rate limited as defined by the Fair Usage Policy throttle rate.

Configuring Data Volume Quotas
==============================
Global volume quotas are configured in the config file.

**Example UCI configuration options:**

Values are in kB.

If set to 0, there is no limit.

Integer values only.

``option uploadquota '0'``

``option downloadquota '0'``

Note: upload means to the Internet, download means from the Internet

**Quotas for individual clients** will override configured global values and are set either by BinAuth or the Authmon Daemon (fas_secure_enable level 3) - see example BinAuth script (binauth_log.sh) and example FAS script (fas-aes-https.php).

Fair Usage Policy Throttle Rate
===============================

If Volume quota is set, an upload/download throttle rate can be configured

Defaults 0

Integer values only

Values are in kb/s

If set to 0, the client will be deauthenticated when the volume quota is exceeded

``option fup_upload_throttle_rate '0'``

``option fup_download_throttle_rate '0'``

Data Rate Quota Threshold
=========================

Both upload and download data rate quotas are a threshold above which traffic is rate limited using a dynamic bucket filter.

There are no additional packages required or further dependencies.

The client data rate is calculated using a moving average.

If the client's average rate exceeds the set quota, packets will be queued using a dynamic bucket filter. This allows clients to burst at a rate exceeding the set quota for a short interval thus improving the user experience compared with a fixed ceiling rate limit.

Upload and download rate limiting is provided independently for upload and download traffic.

The configured rate quotas can be overridden on a client by client basis according to criteria determined by the FAS.

The moving average window size is equal to ratecheckwindow times checkinterval (seconds).

The default value of ratecheckwindow is 2 and is a good working setting. Increasing the value allows a longer period of bursting. Setting to 0 disables all Data Rate Quotas.

Data Rate Quotas are ideal for allowing opening of web pages and emails etc in the fastest way possible, yet preventing an individual client from monopolizing all the available bandwidth by streaming or transferring large files.

Configuring Data Rate Quotas
============================
Data Rate quota thresholds are configured in the config file along with  options that allow tuning of the bursting intervals and bucket filter sizes to suit a wide range of venue requirements.

The configuration options and their defaults are as follows:

* ratecheckwindow (Default 2)
* download_bucket_ratio (Default 10)
* upload_bucket_ratio (Default 10)
* max_download_bucket_size (Default 250)
* max_upload_bucket_size (Default 250)
* download_unrestricted_bursting (Default 0, disabled)
* upload_unrestricted_bursting (Default 0, disabled)
* downloadrate (Default 0, unlimited)
* uploadrate (Default 0, unlimited)

See the Configuration Options section for details.


Note: upload means to the Internet, download means from the Internet

Data Rate Quotas for Individual Clients
---------------------------------------
**Data Rate Quotas for individual clients** will override configured global values and are set by ThemeSpec, BinAuth or the Authmon Daemon.

FAS Levels 3 and 4
==================
FAS levels 3 and 4 use the Authmon daemon to set quota values determined by the FAS. The example scripts fas-hid-https.php and fas-aes-https.php show how to implement this.

FAS level 0, 1 and 2
====================
These levels can either use BinAuth to set quota values determined by the FAS, or if local to the router can use ndsctl authentication with quota values passed as arguments.

If using BinAuth, the FAS would utilise the BinAuth custom variable to send quota values to a BinAuth script configured to interpret the data passed to it in the variable. There is no set method for doing this, it is left to individual installers to develop their own method.

Traffic Shaping
***************

If a fixed ceiling data rate is required, third party traffic shaping packages can be used in place of the built in openNDS Rate Quota Thresholds.
