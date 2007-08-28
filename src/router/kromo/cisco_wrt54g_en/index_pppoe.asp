<div class="setting">
	<div class="label"><% tran("share.usrname"); %></div>
	<input name="ppp_username" size="40" maxlength="63" onblur="valid_name(this,share.usrname)" value="<% nvram_get("ppp_username"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.passwd"); %></div>
	<input id="ppp_passwd" name="ppp_passwd" size="40" maxlength="63" onblur="valid_name(this,share.passwd)" type="password" value="<% nvram_get("ppp_passwd"); %>" />&nbsp;&nbsp;&nbsp;
	<input type="checkbox" name="_ppp_passwd_unmask" value="0" onclick="setElementMask('ppp_passwd', this.checked)" >&nbsp;<% tran("share.unmask"); %></input>
</div>
<div class="setting">
	<div class="label"><% tran("share.srv"); %></div>
	<input name="ppp_service" size="40" maxlength="63" onblur="valid_name(this,share.srv)" value="<% nvram_get("ppp_service"); %>" />
</div>
<div class="setting">
	<div class="label"><% tran("share.compression"); %></div>
	<input class="spaceradio" type="radio" name="ppp_compression" value="1" <% nvram_checked("ppp_compression","1"); %> /><% tran("share.enable"); %>&nbsp;
	<input class="spaceradio" type="radio" name="ppp_compression" value="0" <% nvram_checked("ppp_compression","0"); %> /><% tran("share.disable"); %> 
</div>
<div class="setting">
	<div class="label"><% tran("service.pptpd_encry"); %></div>
	<input size="27" maxlength="63" onblur="valid_name(this,service.pptpd_encry)" name="ppp_mppe" value="<% nvram_get("ppp_mppe"); %>" />
</div>

<!--
<div class="setting">
	<div class="label"><% tran("idx_h.con_strgy"); %><br />&nbsp;</div>
	<input class="spaceradio" type="radio" name="ppp_demand" value="1" onclick="ppp_enable_disable(this.form,1)" <% nvram_checked("ppp_demand","1"); %> /><% tran("idx_h.max_idle"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_idletime" onblur="valid_range(this,0,9999,'Idle time')" value="<% nvram_get("ppp_idletime"); %>" />&nbsp;<% tran("share.mins"); %><br />
	<input class="spaceradio" type="radio" name="ppp_demand" value="0" onclick="ppp_enable_disable(this.form,0)" <% nvram_checked("ppp_demand","0"); %> /><% tran("idx_h.alive"); %>&nbsp;<input class="num" size="4" maxlength="4" name="ppp_redialperiod" onblur="valid_range(this,20,180,'Redial period')" value="<% nvram_get("ppp_redialperiod"); %>" />&nbsp;<% tran("share.secs"); %>
</div>
-->

	<div class="setting">
		<div class="label"><% tran("idx_h.reconnect"); %></div>
		<input class="spaceradio" type="radio" value="1" name="reconnect_enable" <% nvram_checked("reconnect_enable","1"); %> onclick="show_layer_ext(this, 'idreconnect', true)" /><% tran("share.enable"); %>&nbsp;
		<input class="spaceradio" type="radio" value="0" name="reconnect_enable" <% nvram_checked("reconnect_enable","0"); %> onclick="show_layer_ext(this, 'idreconnect', false)" /><% tran("share.disable"); %>
	</div>
	<div id="idreconnect">
		<div class="setting">
			<div class="label"><% tran("share.time"); %></div>
			<select name="reconnect_hours">
				<option value="0" <% nvram_selected("reconnect_hours","0"); %>>00</option>
				<option value="1" <% nvram_selected("reconnect_hours","1"); %>>01</option>
				<option value="2" <% nvram_selected("reconnect_hours","2"); %>>02</option>
				<option value="3" <% nvram_selected("reconnect_hours","3"); %>>03</option>
				<option value="4" <% nvram_selected("reconnect_hours","4"); %>>04</option>
				<option value="5" <% nvram_selected("reconnect_hours","5"); %>>05</option>
				<option value="6" <% nvram_selected("reconnect_hours","6"); %>>06</option>
				<option value="7" <% nvram_selected("reconnect_hours","7"); %>>07</option>
				<option value="8" <% nvram_selected("reconnect_hours","8"); %>>08</option>
				<option value="9" <% nvram_selected("reconnect_hours","9"); %>>09</option>
				<option value="10" <% nvram_selected("reconnect_hours","10"); %>>10</option>
				<option value="11" <% nvram_selected("reconnect_hours","11"); %>>11</option>
				<option value="12" <% nvram_selected("reconnect_hours","12"); %>>12</option>
				<option value="13" <% nvram_selected("reconnect_hours","13"); %>>13</option>
				<option value="14" <% nvram_selected("reconnect_hours","14"); %>>14</option>
				<option value="15" <% nvram_selected("reconnect_hours","15"); %>>15</option>
				<option value="16" <% nvram_selected("reconnect_hours","16"); %>>16</option>
				<option value="17" <% nvram_selected("reconnect_hours","17"); %>>17</option>
				<option value="18" <% nvram_selected("reconnect_hours","18"); %>>18</option>
				<option value="19" <% nvram_selected("reconnect_hours","19"); %>>19</option>
				<option value="20" <% nvram_selected("reconnect_hours","20"); %>>20</option>
				<option value="21" <% nvram_selected("reconnect_hours","21"); %>>21</option>
				<option value="22" <% nvram_selected("reconnect_hours","22"); %>>22</option>
				<option value="23" <% nvram_selected("reconnect_hours","23"); %>>23</option>
			</select>:<select name="reconnect_minutes">
				<option value="0" <% nvram_selected("reconnect_minutes","0"); %>>00</option>
				<option value="1" <% nvram_selected("reconnect_minutes","1"); %>>01</option>
				<option value="2" <% nvram_selected("reconnect_minutes","2"); %>>02</option>
				<option value="3" <% nvram_selected("reconnect_minutes","3"); %>>03</option>
				<option value="4" <% nvram_selected("reconnect_minutes","4"); %>>04</option>
				<option value="5" <% nvram_selected("reconnect_minutes","5"); %>>05</option>
				<option value="6" <% nvram_selected("reconnect_minutes","6"); %>>06</option>
				<option value="7" <% nvram_selected("reconnect_minutes","7"); %>>07</option>
				<option value="8" <% nvram_selected("reconnect_minutes","8"); %>>08</option>
				<option value="9" <% nvram_selected("reconnect_minutes","9"); %>>09</option>
				<option value="10" <% nvram_selected("reconnect_minutes","10"); %>>10</option>
				<option value="11" <% nvram_selected("reconnect_minutes","11"); %>>11</option>
				<option value="12" <% nvram_selected("reconnect_minutes","12"); %>>12</option>
				<option value="13" <% nvram_selected("reconnect_minutes","13"); %>>13</option>
				<option value="14" <% nvram_selected("reconnect_minutes","14"); %>>14</option>
				<option value="15" <% nvram_selected("reconnect_minutes","15"); %>>15</option>
				<option value="16" <% nvram_selected("reconnect_minutes","16"); %>>16</option>
				<option value="17" <% nvram_selected("reconnect_minutes","17"); %>>17</option>
				<option value="18" <% nvram_selected("reconnect_minutes","18"); %>>18</option>
				<option value="19" <% nvram_selected("reconnect_minutes","19"); %>>19</option>
				<option value="20" <% nvram_selected("reconnect_minutes","20"); %>>20</option>
				<option value="21" <% nvram_selected("reconnect_minutes","21"); %>>21</option>
				<option value="22" <% nvram_selected("reconnect_minutes","22"); %>>22</option>
				<option value="23" <% nvram_selected("reconnect_minutes","23"); %>>23</option>
				<option value="24" <% nvram_selected("reconnect_minutes","24"); %>>24</option>
				<option value="25" <% nvram_selected("reconnect_minutes","25"); %>>25</option>
				<option value="26" <% nvram_selected("reconnect_minutes","26"); %>>26</option>
				<option value="27" <% nvram_selected("reconnect_minutes","27"); %>>27</option>
				<option value="28" <% nvram_selected("reconnect_minutes","28"); %>>28</option>
				<option value="29" <% nvram_selected("reconnect_minutes","29"); %>>29</option>
				<option value="30" <% nvram_selected("reconnect_minutes","30"); %>>30</option>
				<option value="31" <% nvram_selected("reconnect_minutes","31"); %>>31</option>
				<option value="32" <% nvram_selected("reconnect_minutes","32"); %>>32</option>
				<option value="33" <% nvram_selected("reconnect_minutes","33"); %>>33</option>
				<option value="34" <% nvram_selected("reconnect_minutes","34"); %>>34</option>
				<option value="35" <% nvram_selected("reconnect_minutes","35"); %>>35</option>
				<option value="36" <% nvram_selected("reconnect_minutes","36"); %>>36</option>
				<option value="37" <% nvram_selected("reconnect_minutes","37"); %>>37</option>
				<option value="38" <% nvram_selected("reconnect_minutes","38"); %>>38</option>
				<option value="39" <% nvram_selected("reconnect_minutes","39"); %>>39</option>
				<option value="40" <% nvram_selected("reconnect_minutes","40"); %>>40</option>
				<option value="41" <% nvram_selected("reconnect_minutes","41"); %>>41</option>
				<option value="42" <% nvram_selected("reconnect_minutes","42"); %>>42</option>
				<option value="43" <% nvram_selected("reconnect_minutes","43"); %>>43</option>
				<option value="44" <% nvram_selected("reconnect_minutes","44"); %>>44</option>
				<option value="45" <% nvram_selected("reconnect_minutes","45"); %>>45</option>
				<option value="46" <% nvram_selected("reconnect_minutes","46"); %>>46</option>
				<option value="47" <% nvram_selected("reconnect_minutes","47"); %>>47</option>
				<option value="48" <% nvram_selected("reconnect_minutes","48"); %>>48</option>
				<option value="49" <% nvram_selected("reconnect_minutes","49"); %>>49</option>
				<option value="50" <% nvram_selected("reconnect_minutes","50"); %>>50</option>
				<option value="51" <% nvram_selected("reconnect_minutes","51"); %>>51</option>
				<option value="52" <% nvram_selected("reconnect_minutes","52"); %>>52</option>
				<option value="53" <% nvram_selected("reconnect_minutes","53"); %>>53</option>
				<option value="54" <% nvram_selected("reconnect_minutes","54"); %>>54</option>
				<option value="55" <% nvram_selected("reconnect_minutes","55"); %>>55</option>
				<option value="56" <% nvram_selected("reconnect_minutes","56"); %>>56</option>
				<option value="57" <% nvram_selected("reconnect_minutes","57"); %>>57</option>
				<option value="58" <% nvram_selected("reconnect_minutes","58"); %>>58</option>
				<option value="59" <% nvram_selected("reconnect_minutes","59"); %>>59</option>
			</select>
		</div>
	</div>
