<!--##################################################################
# This program is free software; you can redistribute it and/or      #
# modify it under the terms of the GNU General Public License as     #
# published by the Free Software Foundation; either version 3 of the #
# License, or (at your option) any later version.                    #
#                                                                    #
# This program is distributed in the hope that it will be useful,    #
# but WITHOUT ANY WARRANTY; without even the implied warranty of     #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      #
# GNU General Public License for more details.                       #
#                                                                    #
# You should have received a copy of the GNU General Public License  #
# along with this program; if not, write to the                      #
# Free Software Foundation, Inc.,                                    #
# 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA             #
######################################################################
# This file is part of a Milkfish Embedded OpenSER SIP Router Setup  #
# and was derived from similar files in this folder by other authors #
#                                                                    #
# The Milkfish-dd Web Interface - Local Aliases Page                 #
#                                                                    #
# Built/Version:  20071229                                           #
# Co-Author/Contact: Michael Poehnl <budrus@sipwerk.com>             #
# Copyright (C) 2007 partly by sipwerk - All rights reserved.        #
#                                                                    #
# Please note that this software is under development and comes with #
# absolutely no warranty, to the extend permitted by applicable law. #
###################################################################-->
<% do_pagehead("bmenu.servicesMilkfish"); %>
		<script type="text/javascript">
		//<![CDATA[

function to_submit(F) {
	F.save_button.value = sbutton.saving;
	apply(F);
}
function to_apply(F) {
	F.save_button.value = sbutton.saving;
	applytake(F);
}
function milkfish_alias_add_submit(F) {
        F.change_action.value="gozila_cgi";
        F.submit_type.value = "add_milkfish_alias";
        F.submit();
}

function milkfish_alias_remove_submit(F) {
        F.change_action.value="gozila_cgi";
        F.submit_type.value = "remove_milkfish_alias";
        F.submit();
}


		</script>
	</head>
	
	<body>
		<form name="setup" action="applyuser.cgi" method="post">
		<input type="hidden" name="submit_button" value="Milkfish_aliases" />
		<input type="hidden" name="action" value="Apply" />
		<input type="hidden" name="change_action" />
		<input type="hidden" name="submit_type" />
		<input type="hidden" name="commit" value="1" />
                <input type="hidden" name="milkfish_ddaliases" /> 
			<div id="main">
				<div id="contentsInfo">
					<h2><% tran("service.milkfish_database"); %></h2>
					<br />
        <fieldset>
                <legend><% tran("service.milkfish_aliases"); %></legend>
                        <table class="table center" summary="chap secrets table">
                        <tr>
                                <th width="30%"><% tran("service.milkfish_alias"); %></th>
                                <th width="30%"><% tran("service.milkfish_uri"); %></th>
                        </tr>
                        <% exec_show_aliases(); %> 
                        </table><br />
                        <div class="center">
                                <script type="text/javascript">
                                //<![CDATA[
                                   document.write("<input class=\"button\" type=\"button\" name=\"add_button\" value=\"" + sbutton.add + "\" onclick=\"milkfish_alias_add_submit(this.form);\" />"); 
                                   document.write("<input class=\"button\" type=\"button\" name=\"del_button\" value=\"" + sbutton.remove + "\" onclick=\"milkfish_alias_remove_submit(this.form);\" />");
//]]>
                                </script>
                        </div>
        </fieldset> 


					<div class="submitFooter">
						<script type="text/javascript">
						//<![CDATA[
						submitFooterButton(1,1,0,0,0,1);
						//]]>
						</script>
					</div>
				</div>
			</div>
		</form>
	</body>
</html>
