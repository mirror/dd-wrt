<div class="setting">
	<div class="label"><% tran("share.annex"); %></div>
	<select name="annex">
		<option value="a" <% nvram_selmatch("annex", "a", "selected"); %>>Annex A</option>
		<option value="at1" <% nvram_selmatch("annex", "at1", "selected"); %>>Annex A T1</option>
		<option value="alite" <% nvram_selmatch("annex", "alite", "selected"); %>>Annex A Lite</option>
		<option value="admt" <% nvram_selmatch("annex", "admt", "selected"); %>>Annex A DMT</option>
		<option value="aadsl2" <% nvram_selmatch("annex", "aadsl2", "selected"); %>>Annex A ADSL2</option>
		<option value="aadsl2+" <% nvram_selmatch("annex", "aadsl2+", "selected"); %>>Annex A ADSL2+</option>
		<option value="l" <% nvram_selmatch("annex", "l", "selected"); %>>Annex L</option>
		<option value="b" <% nvram_selmatch("annex", "b", "selected"); %>>Annex B</option>
		<option value="bdmt" <% nvram_selmatch("annex", "bdmt", "selected"); %>>Annex B DMT</option>
		<option value="badsl2" <% nvram_selmatch("annex", "badsl2", "selected"); %>>Annex B ADSL2</option>
		<option value="badsl2+" <% nvram_selmatch("annex", "badsl2+", "selected"); %>>Annex B ADSL2+</option>
		<option value="m" <% nvram_selmatch("annex", "m", "selected"); %>>Annex M</option>
		<option value="madsl2" <% nvram_selmatch("annex", "madsl2", "selected"); %>>Annex M ADSL2</option>
		<option value="madsl2+" <% nvram_selmatch("annex", "madsl2+", "selected"); %>>Annex M ADSL2+</option>
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
