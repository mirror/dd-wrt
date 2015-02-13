#!/bin/sh
# file: plc/plc.sh

# ====================================================================
# programs;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -o ampboot ampboot.c 
gcc -Wall -Wextra -Wno-unused-parameter -o amphost amphost.c 
gcc -Wall -Wextra -Wno-unused-parameter -o ampID ampID.c
gcc -Wall -Wextra -Wno-unused-parameter -o amplist amplist.c 
gcc -Wall -Wextra -Wno-unused-parameter -o amptest amptest.c
gcc -Wall -Wextra -Wno-unused-parameter -o amprate amprate.c 
gcc -Wall -Wextra -Wno-unused-parameter -o amprate amprule.c 
gcc -Wall -Wextra -Wno-unused-parameter -o ampstat ampstat.c 
gcc -Wall -Wextra -Wno-unused-parameter -o amptool amptool.c 
gcc -Wall -Wextra -Wno-unused-parameter -o amptone amptone.c
gcc -Wall -Wextra -Wno-unused-parameter -o ampwait ampwait.c 
gcc -Wall -Wextra -Wno-unused-parameter -o CMEncrypt CMEncrypt.c
gcc -Wall -Wextra -Wno-unused-parameter -o coqos_add coqos_add.c
gcc -Wall -Wextra -Wno-unused-parameter -o coqos_info coqos_info.c
gcc -Wall -Wextra -Wno-unused-parameter -o coqos_man coqos_man.c
gcc -Wall -Wextra -Wno-unused-parameter -o coqos_mod coqos_mod.c
gcc -Wall -Wextra -Wno-unused-parameter -o coqos_rel coqos_rel.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6k int6k.c 
gcc -Wall -Wextra -Wno-unused-parameter -o int6kboot int6kboot.c 
gcc -Wall -Wextra -Wno-unused-parameter -o int6keth int6keth.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6kmdio int6kmdio.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6kmod int6kmod.c
gcc -Wall -Wextra -Wno-unused-parameter -o int64host int64host.c 
gcc -Wall -Wextra -Wno-unused-parameter -o int6klist int6klist.c 
gcc -Wall -Wextra -Wno-unused-parameter -o int6klog int6klog.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6krate int6krate.c 
gcc -Wall -Wextra -Wno-unused-parameter -o int6krule int6krule.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6kstat int6kstat.c 
gcc -Wall -Wextra -Wno-unused-parameter -o int6ktest int6ktest.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6ktone int6ktone.c
gcc -Wall -Wextra -Wno-unused-parameter -o int6kwait int6kwait.c 
gcc -Wall -Wextra -Wno-unused-parameter -o mdustats mdustats.c
gcc -Wall -Wextra -Wno-unused-parameter -o sada sada.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcotst plcotst.c 
gcc -Wall -Wextra -Wno-unused-parameter -o plchostd plchostd.c -DINT6x00
gcc -Wall -Wextra -Wno-unused-parameter -o plchostd plchostd.c -DAR7x00
gcc -Wall -Wextra -Wno-unused-parameter -o plchost plchost.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcID plcID.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcrate plcrate.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcrule plcrule.c 
gcc -Wall -Wextra -Wno-unused-parameter -o plctool plctool.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcboot plcboot.c
gcc -Wall -Wextra -Wno-unused-parameter -o plctest plctest.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcwait plcwait.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcfwd plcfwd.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcget plcget.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcset plcset.c
gcc -Wall -Wextra -Wno-unused-parameter -o plclog plclog.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcmdio16 plcmdio16.c
gcc -Wall -Wextra -Wno-unused-parameter -o plcmdio32 plcmdio32.c
gcc -Wall -Wextra -Wno-unused-parameter -o plclist plclist.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c Attributes.c
gcc -Wall -Wextra -Wno-unused-parameter -c chipset.c
gcc -Wall -Wextra -Wno-unused-parameter -c Confirm.c
gcc -Wall -Wextra -Wno-unused-parameter -c DeviceIdent.c
gcc -Wall -Wextra -Wno-unused-parameter -c ChangeIdent.c
gcc -Wall -Wextra -Wno-unused-parameter -c Devices.c
gcc -Wall -Wextra -Wno-unused-parameter -c Display.c
gcc -Wall -Wextra -Wno-unused-parameter -c EmulateHost.c
gcc -Wall -Wextra -Wno-unused-parameter -c EmulateHost64.c
gcc -Wall -Wextra -Wno-unused-parameter -c FactoryDefaults.c
gcc -Wall -Wextra -Wno-unused-parameter -c Failure.c
gcc -Wall -Wextra -Wno-unused-parameter -c FlashDevice1.c
gcc -Wall -Wextra -Wno-unused-parameter -c FlashDevice2.c
gcc -Wall -Wextra -Wno-unused-parameter -c FlashMOD.c
gcc -Wall -Wextra -Wno-unused-parameter -c FlashNVM.c
gcc -Wall -Wextra -Wno-unused-parameter -c GetProperty.c
gcc -Wall -Wextra -Wno-unused-parameter -c HostActionIndicate.c
gcc -Wall -Wextra -Wno-unused-parameter -c HostActionResponse.c
gcc -Wall -Wextra -Wno-unused-parameter -c Identity1.c
gcc -Wall -Wextra -Wno-unused-parameter -c Identity2.c
gcc -Wall -Wextra -Wno-unused-parameter -c LinkStatistics.c
gcc -Wall -Wextra -Wno-unused-parameter -c ListLocalDevices.c
gcc -Wall -Wextra -Wno-unused-parameter -c MDUTrafficStats.c
gcc -Wall -Wextra -Wno-unused-parameter -c ModuleDump.c
gcc -Wall -Wextra -Wno-unused-parameter -c ModuleRead.c
gcc -Wall -Wextra -Wno-unused-parameter -c ModuleSpec.c
gcc -Wall -Wextra -Wno-unused-parameter -c ModuleSession.c
gcc -Wall -Wextra -Wno-unused-parameter -c ModuleWrite.c
gcc -Wall -Wextra -Wno-unused-parameter -c ModuleCommit.c
gcc -Wall -Wextra -Wno-unused-parameter -c NVRAMInfo.c
gcc -Wall -Wextra -Wno-unused-parameter -c NVMSelect.c
gcc -Wall -Wextra -Wno-unused-parameter -c LocalDevices.c
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkDevices.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkDevices1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkDevices2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetInfo.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetInfo1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetInfo2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkInformation.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkInformation1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkInformation2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkTraffic.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkTraffic1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c NetworkTraffic2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c ParseRule.c
gcc -Wall -Wextra -Wno-unused-parameter -c PhyRates1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c PhyRates2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c PushButton.c 
gcc -Wall -Wextra -Wno-unused-parameter -c ReadFMI.c
gcc -Wall -Wextra -Wno-unused-parameter -c ReadMFG.c
gcc -Wall -Wextra -Wno-unused-parameter -c ReadMME.c
gcc -Wall -Wextra -Wno-unused-parameter -c ReadFirmware1.c
gcc -Wall -Wextra -Wno-unused-parameter -c ReadParameters.c
gcc -Wall -Wextra -Wno-unused-parameter -c ReadParameters1.c
gcc -Wall -Wextra -Wno-unused-parameter -c ReadParameters2.c
gcc -Wall -Wextra -Wno-unused-parameter -c ReadParameterBlock.c
gcc -Wall -Wextra -Wno-unused-parameter -c Request.c
gcc -Wall -Wextra -Wno-unused-parameter -c ResetAndWait.c
gcc -Wall -Wextra -Wno-unused-parameter -c ResetDevice.c
gcc -Wall -Wextra -Wno-unused-parameter -c RxRates1.c
gcc -Wall -Wextra -Wno-unused-parameter -c RxRates2.c
gcc -Wall -Wextra -Wno-unused-parameter -c rules.c
gcc -Wall -Wextra -Wno-unused-parameter -c SDRAMInfo.c
gcc -Wall -Wextra -Wno-unused-parameter -c SendMME.c
gcc -Wall -Wextra -Wno-unused-parameter -c SetNMK.c
gcc -Wall -Wextra -Wno-unused-parameter -c SetProperty.c
gcc -Wall -Wextra -Wno-unused-parameter -c SignalToNoise1.c
gcc -Wall -Wextra -Wno-unused-parameter -c SignalToNoise2.c
gcc -Wall -Wextra -Wno-unused-parameter -c SlaveMembership.c
gcc -Wall -Wextra -Wno-unused-parameter -c StartFirmware.c
gcc -Wall -Wextra -Wno-unused-parameter -c ToneMaps1.c
gcc -Wall -Wextra -Wno-unused-parameter -c ToneMaps2.c
gcc -Wall -Wextra -Wno-unused-parameter -c Topology.c 
gcc -Wall -Wextra -Wno-unused-parameter -c Topology1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c Topology2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c Antiphon.c 
gcc -Wall -Wextra -Wno-unused-parameter -c Traffic.c 
gcc -Wall -Wextra -Wno-unused-parameter -c Traffic1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c Traffic2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c VersionInfo1.c
gcc -Wall -Wextra -Wno-unused-parameter -c VersionInfo2.c
gcc -Wall -Wextra -Wno-unused-parameter -c WaitForReset.c
gcc -Wall -Wextra -Wno-unused-parameter -c WaitForStart.c
gcc -Wall -Wextra -Wno-unused-parameter -c WatchdogReport.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteCFG.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteMEM.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteMOD.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteNVM.c
gcc -Wall -Wextra -Wno-unused-parameter -c WritePIB.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c BootDevice.c 
gcc -Wall -Wextra -Wno-unused-parameter -c BootDevice1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c BootDevice2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c BootParameters1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c BootParameters2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c BootFirmware1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c BootFirmware2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c InitDevice.c
gcc -Wall -Wextra -Wno-unused-parameter -c InitDevice1.c 
gcc -Wall -Wextra -Wno-unused-parameter -c InitDevice2.c 
gcc -Wall -Wextra -Wno-unused-parameter -c FlashDevice1.c
gcc -Wall -Wextra -Wno-unused-parameter -c FlashDevice2.c
gcc -Wall -Wextra -Wno-unused-parameter -c FlashSoftloader.c
gcc -Wall -Wextra -Wno-unused-parameter -c FlashFirmware.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteFirmware.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteFirmware1.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteFirmware2.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteParameters.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteParameters1.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteParameters2.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteExecuteFirmware.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteExecuteFirmware1.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteExecuteFirmware2.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteExecuteParameters.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteExecuteParameters1.c
gcc -Wall -Wextra -Wno-unused-parameter -c WriteExecuteParameters2.c

# ====================================================================
# functions;
# --------------------------------------------------------------------

gcc -Wall -Wextra -Wno-unused-parameter -c PLCTopology.c -DINT6x00
gcc -Wall -Wextra -Wno-unused-parameter -c PLCTopology.c -DAR7x00
gcc -Wall -Wextra -Wno-unused-parameter -c PLCTopologyPrint.c -DINT6x00
gcc -Wall -Wextra -Wno-unused-parameter -c PLCTopologyPrint.c -DAR7x00
gcc -Wall -Wextra -Wno-unused-parameter -c PLCReadParameterBlock.c

# ====================================================================
# cleanse; 
# --------------------------------------------------------------------

rm -f *.o

