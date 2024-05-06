function __showDistro(element) {
	var selectors = element.parent().children();
	var highlight = element.parent().next();

	selectors.each(function() {
		var sthis = $(this);

		if (sthis.is(element)) {
			sthis.addClass("selected");
			highlight.show();
		} else {
			sthis.removeClass("selected");
			highlight.hide();
		}

		highlight = highlight.next();
	});
}

function showDistro(element) {
	__showDistro($(element));
}

function initDistroSpecificCode() {
	$(".distro-menu").each(function() {
		__showDistro($(this).find(":first-child"));
	});
}

// Second version. Cleaner, but assumes all selectors will have the same options.
function initSelectors() {
	$(".selector-menu").children(":first-child").addClass("selected");
	$(".selector-items").children(":not(:first-child)").hide();

	$(".selector-menu").children().click(function() {
		var selected = $(this).index() + 1;

		// menu
		$(".selector-menu").children().removeClass("selected");
		$(".selector-menu :nth-child(" + selected + ")").addClass("selected");

		// items
		$(".selector-items").children().hide();
		$(".selector-items :nth-child(" + selected + ")").show();
	});
}

window.onload = function() {
	initDistroSpecificCode();
	initSelectors();
};
