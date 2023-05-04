#ifdef SNORT_RELOAD
void AdjustSoRuleMemory(SnortConfig *new_config, SnortConfig *old_config);
void ReloadDynamicDetectionLibs(SnortConfig *sc);
int InitDynamicDetectionPlugins(SnortConfig *sc);
#endif
