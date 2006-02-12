function view_help(url) {
	var win_width = Math.max(Math.floor(screen.width * .3), 380);
	var win_height = Math.max(Math.floor(screen.height * .6), 520);
	var help_win = self.open('./help/' + url + '.asp','DDWRT_Help','alwaysRaised,resizable,scrollbars,width=' + win_width + ',height=' + win_height);
	help_win.focus();
}

// use <a href="javascript:view_help('HSetup');">