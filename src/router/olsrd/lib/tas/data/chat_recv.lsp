<?lua

 tas.keep_state()

 tas.add_header_line("Refresh: 5");

 if not channel or channel == "" then
  channel = "Open"
 end

 if not nick or nick == "" then
  nick = "OLSR User"
 end

 if not list then
  list = {}
 end

 if not listLen then
  listLen = 0
 end

 maxListLen = 25
 httpPort = 1979

 while true do
  msg, from = tas.olsr_get_message("chat00")

  if not msg then break end

  msg = string.gsub(msg, "&", "&amp;")
  msg = string.gsub(msg, "<", "&lt;")
  msg = string.gsub(msg, ">", "&gt;")

  if listLen == maxListLen then
   for i = 0, maxListLen - 2 do
    list[i] = list[i + 1]
   end

   listLen = listLen - 1
  end

  _, _, channel2, nick2, message = string.find(msg, "([^~]*)~([^~]*)~([^~]*)")

  if string.lower(channel) == string.lower(channel2) then

   list[listLen] = {}
   list[listLen]["from"] = from
   list[listLen]["channel"] = channel2
   list[listLen]["nick"] = nick2
   list[listLen]["message"] = message

   listLen = listLen + 1

  end

 end

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
  <head>
    <title>
      OLSR Chat Receive Window
    </title>
  </head>
  <body>

<?lua

 for i = 0, listLen - 1 do

  if i == listLen - 1 then

?>

 <a name="bottom"></a>

<?lua

  end

?>

    <p>
      <b><?lua tas.write(list[i]["nick"]) ?></b>
      (<a href="http://<?lua tas.write(list[i]["from"]) ?>:<?lua tas.write(httpPort) ?>/pub/profile.html" target="_profile"><?lua tas.write(list[i]["from"]) ?></a>),
      <?lua tas.write(list[i]["channel"]) ?>:
      <?lua tas.write(list[i]["message"]) ?>
    </p>

<?lua

 end

?>

    <form>

  </body>
</html>
