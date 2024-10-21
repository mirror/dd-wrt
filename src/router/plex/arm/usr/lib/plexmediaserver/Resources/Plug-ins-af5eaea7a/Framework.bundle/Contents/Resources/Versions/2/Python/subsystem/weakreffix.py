import weakref

_proxy = weakref.proxy
def proxy(obj, callback=None):
  return obj if isinstance(obj, weakref.ProxyTypes) else _proxy(obj, callback)
weakref.proxy = proxy
