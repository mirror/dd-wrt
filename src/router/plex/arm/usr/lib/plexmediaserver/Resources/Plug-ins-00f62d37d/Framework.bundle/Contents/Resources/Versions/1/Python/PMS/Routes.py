import routes
import PMS

routemap = routes.Mapper()
routegen = routes.URLGenerator(routemap, {})
controllers = {}

def ConnectRoute(path, action="__NONE__", **kwargs):
  def ConnectRoute_inner(f):
    if f.__name__ not in controllers:
      controllers[f.__name__] = f
    routemap.connect(None, path, controller=f.__name__, action=action, **kwargs)
    PMS.Log("(Framework) Created a new route: %s => %s" % (path, f.__name__))
    return f
  return ConnectRoute_inner
  
def GenerateRoute(f, action='__NONE__', **kwargs):
  return PMS.Plugin.CurrentPrefix() + routegen(controller=f.__name__, action=action, **kwargs)

def MatchRoute(path):
  try:
    d = routemap.match(path)
    if not d:
      raise
    else:
      f = controllers[d["controller"]]
      del d["controller"]
      if d["action"] == "__NONE__": del d["action"]
      return f, d
  except:
    return None, None