import Framework
import random

from base import BaseComponent

class paste(str): pass

_error_alert_messages = [
  "Someone's doing it wrong!",
  "Uh oh... something exploded.",
  "The Internets have broken.",
  "I think someone changed the Internet again.",
  "Ahhh, you guys and your exceptions...",
  "It wasn't me, guys, honest!",
  "Either there's a ferret in the server room chewing through important cables, or someone wrote buggy code.",
  "ALERT! BAD THINGS! EVERYBODY PANIC!!",
  "I've had enough of this. I quit.",
  "Solar flares have knocked some electrons off course.",
]

class Notifications(BaseComponent):

  def post_to_campfire(self, lines, rooms=None):
    try:
      config = self._core.config

      if not config.cf_token:
        return

      if rooms == None:
        rooms = config.cf_rooms.keys()

      for room_key in rooms:
        room = config.cf_rooms[room_key]
        for line in lines:
          self._core.networking.http_request(
            url = 'https://%s.campfirenow.com/room/%s/speak.json' % (config.cf_domain, room),
            headers = {
              'Content-type': 'application/json',
            },
            data = self._core.data.json.to_string(dict(
              message = dict(
                type = 'PasteMessage' if isinstance(line, paste) else 'TextMessage',
                body = line,#.replace('\n', '&#xA;'),
              )
            )),
            basic_auth = (config.cf_token, 'X'),
            immediate = True,
          )

    except:
      self._core.log_exception("Exception when posting to Campfire")
    
  def _post_to_airbrake(self, msg):
    pass


  def error_alert_message(self):
    return random.choice(_error_alert_messages)