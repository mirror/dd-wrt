<div class="setting">
	<div class="label"><% tran("share.annex"); %></div>
	<select name="annex">
		<option value="0" <% nvram_selmatch("annex", "a", "selected"); %>>Annex A</option>
		<option value="1" <% nvram_selmatch("annex", "b", "selected"); %>>Annex B</option>
		<option value="1" <% nvram_selmatch("annex", "m", "selected"); %>>Annex M</option>
	</select>
</div>
<div class="setting">
	<div class="label"><% tran("share.vpi_vci"); %></div>
	<input name="vpi" size="3" maxlength="3" value="<% nvram_get("vpi"); %>" style="text-align: right;" />/
	<input name="vci" size="3" maxlength="3" value="<% nvram_get("vci"); %>" style="text-align: right;" />
</div>
