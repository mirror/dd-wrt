<div class="setting">
	<div class="label"><% tran("share.annex"); %></div>
	<select name="annex">
		<option value="a" <% nvram_selected("annex", "a"); %>>Annex A</option>
		<option value="at1" <% nvram_selected("annex", "at1"); %>>Annex A T1</option>
		<option value="alite" <% nvram_selected("annex", "alite"); %>>Annex A Lite</option>
		<option value="admt" <% nvram_selected("annex", "admt"); %>>Annex A DMT</option>
		<option value="aadsl2" <% nvram_selected("annex", "aadsl2"); %>>Annex A ADSL2</option>
		<option value="aadsl2+" <% nvram_selected("annex", "aadsl2+"); %>>Annex A ADSL2+</option>
		<option value="b" <% nvram_selected("annex", "b"); %>>Annex B</option>
		<option value="bdmt" <% nvram_selected("annex", "bdmt"); %>>Annex B DMT</option>
		<option value="badsl2" <% nvram_selected("annex", "badsl2"); %>>Annex B ADSL2</option>
		<option value="badsl2+" <% nvram_selected("annex", "badsl2+"); %>>Annex B ADSL2+</option>
		<option value="c" <% nvram_selected("annex", "c"); %>>Annex C</option>
		<option value="j" <% nvram_selected("annex", "j"); %>>Annex J</option>
		<option value="jadsl2" <% nvram_selected("annex", "jadsl2"); %>>Annex J ADSL2</option>
		<option value="jadsl2+" <% nvram_selected("annex", "jadsl2+"); %>>Annex J ADSL2+</option>
		<option value="l" <% nvram_selected("annex", "l"); %>>Annex L</option>
		<option value="m" <% nvram_selected("annex", "m"); %>>Annex M</option>
		<option value="madsl2" <% nvram_selected("annex", "madsl2"); %>>Annex M ADSL2</option>
		<option value="madsl2+" <% nvram_selected("annex", "madsl2+"); %>>Annex M ADSL2+</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.vpi_vci"); %></div>
	<input name="vpi" size="3" maxlength="3" value="<% nvram_get("vpi"); %>" style="text-align: right;" />/
	<input name="vci" size="3" maxlength="3" value="<% nvram_get("vci"); %>" style="text-align: right;" />
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
