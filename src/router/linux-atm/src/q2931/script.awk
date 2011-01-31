/^# 1 "[^"]*\/linux\/atmsap.h" / { print substr($3, 2, length($3) - 2); nextfile; }
