<div class="setting">
	<div class="label"><% tran("share.vpi_vci"); %></div>
	<input name="vpi" size="3" maxlength="3" value="<% nvram_get("vpi"); %>" style="text-align: right;" />/
	<input name="vci" size="3" maxlength="3" value="<% nvram_get("vci"); %>" style="text-align: right;" />
</div>
<div class="setting">
	<div class="label"><% tran("share.annex"); %></div>
	<select name="annex">
		<option value="a" <% nvram_selmatch("annex", "a", "selected"); %>>Annex A</option>
		<option value="b" <% nvram_selmatch("annex", "b", "selected"); %>>Annex B</option>
		<option value="m" <% nvram_selmatch("annex", "m", "selected"); %>>Annex M</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.encaps"); %></div>
	<select name="atm_encaps">
		<option value="0" <% nvram_selmatch("atm_encaps", "0", "selected"); %>>LLC</option>
		<option value="1" <% nvram_selmatch("atm_encaps", "1", "selected"); %>>VC Mux</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.payload"); %></div>
	<select name="atm_payld">
		<option value="0" <% nvram_selmatch("atm_payld", "0", "selected"); %>>Routed</option>
		<option value="1" <% nvram_selmatch("atm_payld", "1", "selected"); %>>Bridged</option>
	</select>
</div>
