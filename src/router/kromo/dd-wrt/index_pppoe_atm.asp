<div class="setting">
	<div class="label"><% tran("share.annex"); %></div>
	<select name="annex">
		<option value="a" <% nvs("annex", "a"); %>>Annex A</option>
		<option value="at1" <% nvs("annex", "at1"); %>>Annex A T1</option>
		<option value="alite" <% nvs("annex", "alite"); %>>Annex A Lite</option>
		<option value="admt" <% nvs("annex", "admt"); %>>Annex A DMT</option>
		<option value="aadsl2" <% nvs("annex", "aadsl2"); %>>Annex A ADSL2</option>
		<option value="aadsl2+" <% nvs("annex", "aadsl2+"); %>>Annex A ADSL2+</option>
		<option value="b" <% nvs("annex", "b"); %>>Annex B</option>
		<option value="bdmt" <% nvs("annex", "bdmt"); %>>Annex B DMT</option>
		<option value="badsl2" <% nvs("annex", "badsl2"); %>>Annex B ADSL2</option>
		<option value="badsl2+" <% nvs("annex", "badsl2+"); %>>Annex B ADSL2+</option>
		<option value="c" <% nvs("annex", "c"); %>>Annex C</option>
		<option value="j" <% nvs("annex", "j"); %>>Annex J</option>
		<option value="jadsl2" <% nvs("annex", "jadsl2"); %>>Annex J ADSL2</option>
		<option value="jadsl2+" <% nvs("annex", "jadsl2+"); %>>Annex J ADSL2+</option>
		<option value="l" <% nvs("annex", "l"); %>>Annex L</option>
		<option value="m" <% nvs("annex", "m"); %>>Annex M</option>
		<option value="madsl2" <% nvs("annex", "madsl2"); %>>Annex M ADSL2</option>
		<option value="madsl2+" <% nvs("annex", "madsl2+"); %>>Annex M ADSL2+</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.vpi_vci"); %></div>
	<input name="vpi" size="3" maxlength="3" value="<% nvg("vpi"); %>" style="text-align: right;" />/
	<input name="vci" size="3" maxlength="3" value="<% nvg("vci"); %>" style="text-align: right;" />
</div>
<div class="setting">
	<div class="label"><% tran("share.encaps"); %></div>
	<select name="atm_encaps">
		<option value="0" <% nvsm("atm_encaps", "0", "selected"); %>>LLC</option>
		<option value="1" <% nvsm("atm_encaps", "1", "selected"); %>>VC Mux</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.payload"); %></div>
	<select name="atm_payld">
		<option value="0" <% nvsm("atm_payld", "0", "selected"); %>>Routed</option>
		<option value="1" <% nvsm("atm_payld", "1", "selected"); %>>Bridged</option>
	</select>
</div>
