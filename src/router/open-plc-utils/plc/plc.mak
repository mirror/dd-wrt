# file: plc.mak

# ====================================================================
#
# --------------------------------------------------------------------

ampboot.o: ampboot.c error.h files.h flags.h getoptv.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
amphost.o: amphost.c error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
ampID.o: ampID.c error.h files.h flags.h getoptv.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
amplist.o: amplist.c error.h files.h flags.h getoptv.h plc.h putoptv.h symbol.h types.h
amprate.o: amprate.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
ampstat.o: ampstat.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
amptest.o: amptest.c channel.h error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
amptone.o: amptone.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
amptool.o: amptool.c HPAVKey.h channel.h error.h files.h flags.h getoptv.h keys.h memory.h number.h nvm.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
ampwait.o: ampwait.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h timer.h types.h
chipset.o: chipset.c error.h plc.h symbol.h types.h
coqos_add.o: coqos_add.c coqos.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h rules.h symbol.h types.h
coqos_info.o: coqos_info.c coqos.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
coqos_man.o: coqos_man.c coqos.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
coqos_mod.o: coqos_mod.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
coqos_rel.o: coqos_rel.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
int64host.o: int64host.c error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
int6k.o: int6k.c HPAVKey.h channel.h error.h files.h flags.h getoptv.h keys.h memory.h number.h nvm.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
int6kboot.o: int6kboot.c error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
int6keth.o: int6keth.c channel.h error.h files.h flags.h getoptv.h memory.h mme.h number.h plc.h putoptv.h symbol.h types.h
int6khost.o: int6khost.c error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
int6kid.o: int6kid.c error.h files.h flags.h getoptv.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
int6klist.o: int6klist.c error.h files.h flags.h getoptv.h plc.h putoptv.h symbol.h types.h
int6klog.o: int6klog.c base64.h channel.h error.h files.h flags.h format.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
int6kmain.o: int6kmain.c channel.h error.h files.h flags.h getoptv.h memory.h mme.h number.h plc.h symbol.h types.h
int6kmdio.o: int6kmdio.c error.h files.h flags.h getoptv.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
int6kmdio2.o: int6kmdio2.c error.h files.h flags.h getoptv.h mdio.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
int6kmod.o: int6kmod.c channel.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
int6krate.o: int6krate.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
int6krule.o: int6krule.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h rules.h symbol.h types.h
int6kstat.o: int6kstat.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
int6ktest.o: int6ktest.c channel.h error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
int6ktone.o: int6ktone.c error.h files.h flags.h getoptv.h memory.h plc.h putoptv.h symbol.h types.h
int6kwait.o: int6kwait.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h timer.h types.h
mdustats.o: mdustats.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
piboffset.o: piboffset.c pib.h plc.h
plc.o: plc.c channel.h
plcboot.o: plcboot.c error.h files.h flags.h getoptv.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
plcdevs.o: plcdevs.c channel.h error.h ether.h flags.h getoptv.h memory.h number.h plc.h putoptv.h version.h
plcfwd.o: plcfwd.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plcget.o: plcget.c channel.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plchost.o: plchost.c channel.h error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
plchostd.o: plchostd.c channel.h error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
plcID.o: plcID.c error.h files.h flags.h getoptv.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
plclist.o: plclist.c error.h files.h flags.h getoptv.h plc.h putoptv.h symbol.h types.h
plclog.o: plclog.c base64.h channel.h error.h files.h flags.h format.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plcmdio16.o: plcmdio16.c error.h files.h flags.h getoptv.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
plcmdio32.o: plcmdio32.c error.h files.h flags.h getoptv.h mdio.h memory.h mme.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
plcotst.o: plcotst.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plcrate.o: plcrate.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plcrule.o: plcrule.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h rules.h symbol.h types.h
plcset.o: plcset.c channel.h error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plcstat.o: plcstat.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plctest.o: plctest.c channel.h error.h files.h flags.h getoptv.h memory.h number.h nvm.h nvram.h pib.h plc.h putoptv.h sdram.h types.h
plctone.o: plctone.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h types.h
plctool.o: plctool.c HPAVKey.h channel.h error.h files.h flags.h getoptv.h keys.h memory.h number.h nvm.h pib.h plc.h putoptv.h sdram.h symbol.h types.h
plcwait.o: plcwait.c error.h files.h flags.h getoptv.h memory.h number.h plc.h putoptv.h symbol.h timer.h types.h
rules.o: rules.c rules.h
sada.o: sada.c HPAVKey.h error.h files.h flags.h getoptv.h memory.h number.h nvm.h pib.h plc.h putoptv.h sdram.h symbol.h types.h

# ====================================================================
#
# --------------------------------------------------------------------

Antiphon.o: Antiphon.c error.h flags.h plc.h
Attributes.o: Attributes.c plc.h
Attributes1.o: Attributes1.c error.h flags.h format.h memory.h plc.h
Attributes2.o: Attributes2.c error.h flags.h format.h memory.h plc.h
BootDevice.o: BootDevice.c plc.h
BootDevice1.o: BootDevice1.c plc.h
BootDevice2.o: BootDevice2.c plc.h
BootFirmware1.o: BootFirmware1.c error.h files.h flags.h nvm.h plc.h
BootFirmware2.o: BootFirmware2.c error.h files.h flags.h nvm.h plc.h
BootParameters1.o: BootParameters1.c error.h files.h flags.h pib.h plc.h
BootParameters2.o: BootParameters2.c error.h files.h flags.h pib.h plc.h
CMEncrypt.o: CMEncrypt.c Confirm.c Devices.c EthernetHeader.c Failure.c HomePlugHeader.c MMECode.c Request.c SHA256.h SHA256Block.c SHA256Fetch.c SHA256Reset.c SHA256Write.c channel.c closechannel.c error.c error.h files.h flags.h getoptv.c getoptv.h hexdecode.c hexdump.c hexencode.c memory.h number.h openchannel.c plc.h putoptv.c putoptv.h readpacket.c sendpacket.c symbol.h synonym.c todigit.c types.h uintspec.c version.c
ChangeIdent.o: ChangeIdent.c HPAVKey.h error.h pib.h plc.h
Confirm.o: Confirm.c flags.h memory.h plc.h
DeviceIdent.o: DeviceIdent.c error.h flags.h memory.h pib.h plc.h
Devices.o: Devices.c plc.h types.h
Display.o: Display.c memory.h plc.h
EmulateHost.o: EmulateHost.c channel.h error.h files.h flags.h nvm.h pib.h plc.h
EmulateHost64.o: EmulateHost64.c channel.h error.h files.h flags.h nvm.h pib.h plc.h
ExecuteApplets.o: ExecuteApplets.c plc.h
ExecuteApplets1.o: ExecuteApplets1.c error.h files.h flags.h plc.h
ExecuteApplets2.o: ExecuteApplets2.c error.h files.h flags.h plc.h
FactoryDefaults.o: FactoryDefaults.c error.h memory.h plc.h
FactoryReset.o: FactoryReset.c error.h memory.h plc.h
Failure.o: Failure.c error.h flags.h memory.h mme.h plc.h
FlashDevice1.o: FlashDevice1.c error.h flags.h plc.h
FlashDevice2.o: FlashDevice2.c flags.h plc.h
FlashFirmware.o: FlashFirmware.c plc.h
FlashMOD.o: FlashMOD.c error.h memory.h plc.h
FlashNVM.o: FlashNVM.c error.h memory.h plc.h
FlashParameters.o: FlashParameters.c plc.h
FlashSoftloader.o: FlashSoftloader.c plc.h
FlashUpgrade.o: FlashUpgrade.c plc.h
GetProperty.o: GetProperty.c error.h memory.h number.h plc.h
HostActionIndicate.o: HostActionIndicate.c error.h memory.h plc.h
HostActionResponse.o: HostActionResponse.c error.h memory.h plc.h
Identity.o: Identity.c plc.h
Identity1.o: Identity1.c error.h memory.h pib.h plc.h
Identity2.o: Identity2.c endian.h error.h files.h flags.h memory.h nvm.h plc.h
InitDevice.o: InitDevice.c error.h files.h plc.h
InitDevice1.o: InitDevice1.c error.h files.h flags.h memory.h nvm.h plc.h
InitDevice2.o: InitDevice2.c error.h files.h flags.h memory.h nvm.h plc.h
LinkStatistics.o: LinkStatistics.c error.h memory.h number.h plc.h
LinkStatus.o: LinkStatus.c error.h memory.h symbol.h plc.h
ListLocalDevices.o: ListLocalDevices.c number.h plc.h
ListRemoteDevices.o: ListRemoteDevices.c plc.h
ListRemoteDevices1.o: ListRemoteDevices1.c flags.h number.h plc.h
ListRemoteDevices2.o: ListRemoteDevices2.c flags.h number.h plc.h
LocalDeviceList.o: LocalDeviceList.c number.h plc.h
LocalDevices.o: LocalDevices.c channel.h error.h plc.h types.h
Loopback1.o: Loopback1.c channel.h error.h flags.h memory.h mme.h plc.h
MDUTrafficStats.o: MDUTrafficStats.c error.h flags.h memory.h plc.h
ModuleCommit.o: ModuleCommit.c error.h plc.h
ModuleDump.o: ModuleDump.c error.h plc.h
ModuleRead.o: ModuleRead.c endian.h error.h files.h memory.h plc.h
ModuleSession.o: ModuleSession.c error.h plc.h
ModuleSpec.o: ModuleSpec.c error.h files.h plc.h
ModuleWrite.o: ModuleWrite.c error.h files.h plc.h
NVMSelect.o: NVMSelect.c error.h files.h plc.h
NVRAMInfo.o: NVRAMInfo.c error.h memory.h nvram.h plc.h symbol.h
NetInfo.o: NetInfo.c plc.h
NetInfo1.o: NetInfo1.c error.h memory.h number.h plc.h
NetInfo2.o: NetInfo2.c error.h memory.h number.h plc.h
NetworkDevices.o: NetworkDevices.c plc.h
NetworkDevices1.o: NetworkDevices1.c error.h memory.h number.h plc.h types.h
NetworkDevices2.o: NetworkDevices2.c error.h memory.h number.h plc.h types.h
NetworkInfoStats.o: NetworkInfoStats.c error.h memory.h number.h plc.h
NetworkInformation.o: NetworkInformation.c plc.h
NetworkInformation1.o: NetworkInformation1.c error.h memory.h number.h plc.h
NetworkInformation2.o: NetworkInformation2.c error.h memory.h number.h plc.h
NetworkTraffic.o: NetworkTraffic.c plc.h
NetworkTraffic1.o: NetworkTraffic1.c error.h flags.h memory.h number.h plc.h
NetworkTraffic2.o: NetworkTraffic2.c error.h flags.h memory.h number.h plc.h
PLCHostBoot.o: PLCHostBoot.c error.h files.h flags.h plc.h
PLCNetworkInfo.o: PLCNetworkInfo.c error.h flags.h plc.h types.h
PLCPhyRates.o: PLCPhyRates.c error.h flags.h plc.h types.h
PLCReadParameterBlock.o: PLCReadParameterBlock.c error.h files.h pib.h plc.h
PLCSelect.o: PLCSelect.c plc.h
PLCTopology.o: PLCTopology.c error.h flags.h memory.h plc.h symbol.h
PLCTopologyPrint.o: PLCTopologyPrint.c memory.h plc.h
ParseRule.o: ParseRule.c ether.h error.h memory.h number.h rules.h
PhyRates.o: PhyRates.c plc.h
PhyRates1.o: PhyRates1.c error.h flags.h plc.h types.h
PhyRates2.o: PhyRates2.c error.h flags.h memory.h plc.h
Platform.o: Platform.c channel.h error.h flags.h memory.h plc.h symbol.h
PushButton.o: PushButton.c error.h memory.h plc.h
ReadFMI.o: ReadFMI.c error.h memory.h number.h plc.h
ReadFirmware.o: ReadFirmware.c plc.h
ReadFirmware1.o: ReadFirmware1.c endian.h error.h files.h memory.h nvm.h plc.h
ReadFirmware2.o: ReadFirmware2.c plc.h
readmessage.o: readmessage.c error.h mme.h plc.h
ReadMFG.o: ReadMFG.c channel.h flags.h memory.h plc.h
ReadMME.o: ReadMME.c error.h mme.h plc.h
ReadNVM.o: ReadNVM.c endian.h error.h files.h memory.h nvm.h plc.h
ReadParameterBlock.o: ReadParameterBlock.c error.h files.h pib.h plc.h
ReadParameters.o: ReadParameters.c plc.h
ReadParameters1.o: ReadParameters1.c error.h files.h pib.h plc.h
ReadParameters2.o: ReadParameters2.c plc.h
RemoteDeviceList.o: RemoteDeviceList.c plc.h
RemoteDeviceList1.o: RemoteDeviceList1.c flags.h number.h plc.h
RemoteDeviceList2.o: RemoteDeviceList2.c flags.h number.h plc.h
RemoteHosts.o: RemoteHosts.c error.h memory.h plc.h symbol.h
Request.o: Request.c flags.h memory.h plc.h
ResetAndWait.o: ResetAndWait.c error.h memory.h plc.h
ResetDevice.o: ResetDevice.c error.h memory.h plc.h
RxRates1.o: RxRates1.c error.h flags.h memory.h plc.h
RxRates2.o: RxRates2.c error.h flags.h memory.h plc.h
SDRAMInfo.o: SDRAMInfo.c error.h memory.h nvm.h plc.h sdram.h
SendMME.o: SendMME.c plc.h
SetNMK.o: SetNMK.c HPAVKey.h error.h flags.h memory.h plc.h
SetProperty.o: SetProperty.c error.h memory.h plc.h
SignalToNoise1.o: SignalToNoise1.c endian.h error.h flags.h plc.h
SignalToNoise2.o: SignalToNoise2.c endian.h error.h flags.h mme.h plc.h types.h
SlaveMembership.o: SlaveMembership.c HPAVKey.h error.h memory.h plc.h
StartDevice.o: StartDevice.c error.h files.h nvm.h pib.h plc.h
StartFirmware.o: StartFirmware.c error.h plc.h
StartFirmware1.o: StartFirmware1.c error.h plc.h
StartFirmware2.o: StartFirmware2.c error.h plc.h
StationRole.o: StationRole.c plc.h
ToneMaps1.o: ToneMaps1.c endian.h error.h flags.h plc.h
ToneMaps2.o: ToneMaps2.c endian.h error.h flags.h mme.h plc.h
Topology.o: Topology.c plc.h
Topology1.o: Topology1.c channel.h error.h flags.h memory.h plc.h
Topology2.o: Topology2.c channel.h error.h flags.h memory.h plc.h
Traffic.o: Traffic.c plc.h
Traffic1.o: Traffic1.c error.h flags.h memory.h mme.h plc.h
Traffic2.o: Traffic2.c error.h flags.h memory.h mme.h plc.h
Traffic3.o: Traffic3.c channel.h error.h flags.h memory.h mme.h plc.h
Transmit.o: Transmit.c error.h flags.h plc.h timer.h
VersionInfo1.o: VersionInfo1.c error.h memory.h plc.h symbol.h
VersionInfo2.o: VersionInfo2.c error.h memory.h plc.h symbol.h
WaitForReset.o: WaitForReset.c error.h plc.h timer.h
WaitForStart.o: WaitForStart.c error.h plc.h timer.h
WatchdogReport.o: WatchdogReport.c error.h flags.h format.h memory.h plc.h
WriteCFG.o: WriteCFG.c error.h files.h memory.h plc.h sdram.h
WriteExecuteApplet2.o: WriteExecuteApplet2.c error.h files.h plc.h
WriteExecuteFirmware.o: WriteExecuteFirmware.c plc.h
WriteExecuteFirmware1.o: WriteExecuteFirmware1.c error.h files.h plc.h
WriteExecuteFirmware2.o: WriteExecuteFirmware2.c error.h files.h plc.h
WriteExecuteParameters.o: WriteExecuteParameters.c plc.h
WriteExecuteParameters1.o: WriteExecuteParameters1.c error.h files.h plc.h
WriteExecuteParameters2.o: WriteExecuteParameters2.c error.h files.h plc.h
WriteFirmware.o: WriteFirmware.c plc.h
WriteFirmware1.o: WriteFirmware1.c error.h files.h memory.h plc.h
WriteFirmware2.o: WriteFirmware2.c error.h files.h memory.h plc.h
WriteMEM.o: WriteMEM.c error.h files.h memory.h plc.h
WriteMOD.o: WriteMOD.c error.h files.h memory.h plc.h
WriteNVM.o: WriteNVM.c error.h files.h memory.h plc.h
WritePIB.o: WritePIB.c error.h files.h memory.h plc.h
WriteParameters.o: WriteParameters.c plc.h
WriteParameters1.o: WriteParameters1.c error.h files.h memory.h plc.h
WriteParameters2.o: WriteParameters2.c error.h files.h memory.h plc.h

# ====================================================================
# headers;
# --------------------------------------------------------------------

plc.h: HPAVKey.h channel.h mme.h nvm.h pib.h types.h
rules.h: symbol.h


