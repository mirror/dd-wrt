% Public functions:
%  rline_up_hist_search
%  rline_down_hist_search
%
% These functions search up/down the history list displaying history
% lines that match the starting line.  The idea behind this was inspired
% by the zsh up/down-line-or-search functions.
%

define rline_up_hist_search();
define rline_down_hist_search ();
private variable History_Search = struct
{
   history,
   prefix,
   current_index,
   point,
};

private define get_unique_history (prefix)
{
   variable h = rline_get_history ();

   if (prefix == "")
     return [h, prefix];

   variable i = strncmp (h, prefix, strlen(prefix));
   h = h[wherenot(i)];

   variable new_h = String_Type[0];

   %  build the list using most recent history first
   array_reverse (h);
   foreach (h)
     {
	variable name = ();
	if (any (name == new_h))
	  continue;
	new_h = [new_h, name];
     }
   array_reverse (new_h);
   return [new_h, prefix];
}

private define get_history_search (dir)
{
   variable func = rline_get_last_key_function();

   if ((History_Search.history == NULL)
       || (typeof (func) != Ref_Type)
       || ((func != &rline_up_hist_search) && (func != &rline_down_hist_search)))
     {
	variable point = rline_get_point ();
	History_Search.point = point;
	variable prefix = rline_get_line()[[0:point-1]];
	History_Search.history = get_unique_history (prefix);
	History_Search.current_index = 0;
	if (dir < 0)
	  History_Search.current_index = length(History_Search.history)-1;
	History_Search.prefix = prefix;
     }
   return History_Search;
}

private define up_down_hist_search (dir)
{
   variable h = get_history_search (dir);
   variable num = length (h.history);
   if (num == 0)
     return;

   variable index = h.current_index + dir;

   ifnot (0 <= index < num)
     return;

   rline_delete_line ();
   rline_ins (h.history[index]);
   h.current_index = index;
}

define rline_up_hist_search ()
{
   up_down_hist_search (-1);
}

define rline_down_hist_search ()
{
   up_down_hist_search (1);
}
