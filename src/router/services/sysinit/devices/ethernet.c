
static int try_module(char *module)
{
sysprintf("insmod %s",module); 
return 1;
}
#define insmod(module) returncode=try_module(module);

static int detect(char *devicename)
{
	FILE *tmp = fopen("/tmp/devices", "rb");

	if (tmp == NULL) {
		system2("/sbin/lspci>/tmp/devices");
	} else
		fclose(tmp);
	char devcall[128];
	int res;

	sprintf(devcall, "cat /tmp/devices|/bin/grep \"%s\"|/bin/wc -l",
		devicename);
	FILE *in = popen(devcall, "rb");

	fscanf(in, "%d", &res);
	pclose(in);
	return res > 0 ? 1 : 0;
}

int detect_ethernet_devices(void)
{
	int returncode = 0;
	if (detect("Rhine-"))	// VIA Rhine-I, Rhine-II, Rhine-III
		insmod("via-rhine");
	if (detect("VT6120"))	// VIA Rhine-I, Rhine-II, Rhine-III
		insmod("via-velocity");
	else if (detect("VT6121"))	// VIA Rhine-I, Rhine-II, Rhine-III
		insmod("via-velocity");
	else if (detect("VT6122"))	// VIA Rhine-I, Rhine-II, Rhine-III
		insmod("via-velocity");

	if (detect("DP8381"))
		insmod("natsemi");
	if (detect("PCnet32"))	// vmware?
		insmod("pcnet32");
	if (detect("Tigon3"))	// Broadcom 
		insmod("tg3");
	else if (detect("NetXtreme"))	// Broadcom 
		insmod("tg3");
	if (detect("NetXtreme II"))	// Broadcom 
		insmod("bnx2");
	if (detect("BCM44"))	// Broadcom 
		insmod("b44");

	if (detect("EtherExpress PRO/100"))	// intel 100 mbit 
		insmod("e100");
	else if (detect("PRO/100"))	// intel 100 mbit
		insmod("e100");
	else if (detect("8280"))	// intel 100 mbit 
		insmod("e100");
	else if (detect("Ethernet Pro 100"))	// intel 100 mbit 
		insmod("e100");
	else if (detect("8255"))	// intel 100 mbit 
		insmod("eepro100");

	if (detect("PRO/1000"))	// Intel Gigabit 
	{
		insmod("e1000");
		insmod("e1000e");
	} else if (detect("82541"))	// Intel Gigabit
	{
		insmod("e1000");
		insmod("e1000e");
	} else if (detect("82547"))	// Intel Gigabit
	{
		insmod("e1000");
		insmod("e1000e");
	} else if (detect("82546"))	// Intel Gigabit
	{
		insmod("e1000");
		insmod("e1000e");
	} else if (detect("82545"))	// Intel Gigabit / VMWare 64 bit mode 
	{
		insmod("e1000");
		insmod("e1000e");
	} else if (detect("82543"))	// Intel Gigabit / VMWare 64 bit mode 
	{
		insmod("e1000");
		insmod("e1000e");
	} else if (detect("82572"))	// Intel Gigabit 
	{
		insmod("e1000");
		insmod("e1000e");
	}
	if (detect("Tolapai"))	// Realtek 8169 Adapter (various notebooks) 
	{
		insmod("e1000");
		insmod("e1000e");
		insmod("e1000gcu");
		insmod("e1000gbe");
	} else if (detect("EP80579"))	// Realtek 8169 Adapter (various notebooks) 
	{
		insmod("e1000");
		insmod("e1000e");
		insmod("e1000gcu");
		insmod("e1000gbe");
	}
	if (detect("RTL-8110"))	// Realtek 8169 Adapter (various notebooks) 
		insmod("r8169");
	else if (detect("RTL-8111"))	// Realtek 8169 Adapter (various notebooks) 
		insmod("r8169");
	else if (detect("RTL8111"))	// Realtek 8169 Adapter (various notebooks) 
		insmod("r8169");
	else if (detect("RTL-8169"))	// Realtek 8169 Adapter (various
		// notebooks) 
		insmod("r8169");
	else if (detect("Linksys Gigabit"))
		insmod("r8169");
	else if (detect("RTL8101"))	// Realtek 8169 Adapter (various
		// notebooks) 
		insmod("r8169");

	if (detect("Happy Meal"))
		insmod("sunhme");

	if (detect("8139"))	// Realtek 8139 Adapter (various notebooks) 
		insmod("8139too");
	if (detect("DFE-690TXD"))	// Realtek 8139 Adapter (various
		// notebooks) 
		insmod("8139too");
	else if (detect("SMC2-1211TX"))	// Realtek 8139 Adapter (various
		// notebooks) 
		insmod("8139too");
	else if (detect("Robotics"))	// Realtek 8139 Adapter (various
		// notebooks) 
		insmod("8139too");

	if (detect("nForce2 Ethernet"))	// nForce2 
		insmod("forcedeth");
	else if (detect("nForce3 Ethernet"))	// nForce3 
		insmod("forcedeth");
	else if (detect("nForce Ethernet"))	// nForce 
		insmod("forcedeth");
	else if (detect("CK804 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("CK8S Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP04 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP2A Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP51 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP55 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP61 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP65 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP67 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP67 Gigabit"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP73 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP77 Ethernet"))	// nForce
		insmod("forcedeth");
	else if (detect("MCP79 Ethernet"))	// nForce
		insmod("forcedeth");

	if (detect("Sundance"))	// Dlink fibre
		insmod("sundance");
	else if (detect("DL10050"))
		insmod("sundance");

	if (detect("88E8001"))	// Marvell Yukon
		insmod("sk98lin");
	else if (detect("RDK-"))
		insmod("sk98lin");
	else if (detect("SK-98"))
		insmod("sk98lin");
	else if (detect("3c940"))
		insmod("sk98lin");
	else if (detect("Marvell"))
		insmod("sk98lin");

	if (detect("RTL-8029"))	// Old Realtek PCI NE2000 clone (10M only)
	{
		insmod("8390");
		insmod("ne2k-pci");
	}

	if (detect("3c905"))	// 3Com
		insmod("3c59x");
	else if (detect("3c555"))	// 3Com
		insmod("3c59x");
	else if (detect("3c556"))	// 3Com
		insmod("3c59x");
	else if (detect("ScSOHO100"))	// 3Com
		insmod("3c59x");
	else if (detect("Hurricane"))	// 3Com
		insmod("3c59x");

	if (detect("LNE100TX"))	// liteon / linksys
		insmod("tulip");
	else if (detect("FasterNet"))
		insmod("tulip");
	else if (detect("ADMtek NC100"))
		insmod("tulip");
	else if (detect("910-A1"))
		insmod("tulip");
	else if (detect("tulip"))
		insmod("tulip");
	else if (detect("DECchip 21142"))
		insmod("tulip");
	else if (detect("MX987x5"))
		insmod("tulip");

	if (detect("DGE-530T"))
		insmod("skge");
	else if (detect("D-Link Gigabit"))
		insmod("skge");

	if (detect("SiS900"))	// Sis 900
		insmod("sis900");

	if (detect("SafeXcel-1141")) {
		insmod("ocf");
		insmod("cryptodev");
		insmod("safe");
		nvram_set("use_crypto", "1");
	} else
		nvram_set("use_crypto", "0");

	return returncode;
}
