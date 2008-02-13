<?lua

 tas.keep_state()

 tas.add_header_line("Refresh: 5");

 if not channel or channel == "" then
  channel = "Open"
 end

 if not nick or nick == "" then
  nick = "OLSR User"
 end

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
  <head>
    <title>
      OLSR Chat Head Window
    </title>
  </head>
  <body>

    <h2>User "<?lua tas.write(nick) ?>" in channel "<?lua tas.write(channel) ?>"</h2>

  </body>
</html>
