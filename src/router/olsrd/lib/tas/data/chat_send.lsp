<?lua

 tas.keep_state()

 para = tas.get_parameters()

 message = para["message"]

 if para["channel"] then
  channel = para["channel"]
 end

 if para["nick"] then
  nick = para["nick"]
 end

 if not channel or channel == "" then
  channel = "Open"
 end

 if not nick or nick == "" then
  nick = "OLSR User"
 end 

 if message and message ~= "" then
  channel = string.gsub(channel, "~", "?")
  nick = string.gsub(nick, "~", "?")
  message = string.gsub(message, "~", "?")

  payload = channel.."~"..nick.."~"..message

  tas.olsr_send_message("chat00", payload)
 end

?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
  <head>
    <title>
      OLSR Chat Send Window
    </title>
  </head>
  <body>
    <form method="GET" action="chat_send.lsp">
      <table>
        <tr>
          <td>Channel</td>
          <td><input type="text" name="channel" size="20" value="<?lua tas.write(channel) ?>"></td>
        </tr>
        <tr>
          <td>Nickname</td>
          <td><input type="text" name="nick" size="20" value="<?lua tas.write(nick) ?>"></td>
        </tr>
        <tr>
          <td>Message</td>
          <td><input type="text" name="message" size="80" value=""></td>
          <td><input type="submit" name="ok" value="OK"></td>
        </tr>
      </table>
    </form>
  </body>
</html>
