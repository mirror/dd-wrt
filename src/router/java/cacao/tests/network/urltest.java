import java.net.*;

public class urltest {
    public static void main (String args[])
    {
	String classes[] =
	{
	    "w3c.jigsaw.http.httpd",
	    "sun.net.www.protocol.http.Handler",
	    "w3c.tools.store.AttributeHolder",
	    "w3c.tools.store.ResourceStoreState",
	    "w3c.tools.store.Resource",
	    "w3c.jigsaw.formedit.GenericResourceEditor",
	    "w3c.tools.store.BooleanAttribute",
	    "w3c.jigsaw.forms.BooleanField",
	    "w3c.jigsaw.auth.PasswordAttribute",
	    "w3c.jigsaw.auth.PasswordField",
	    "w3c.www.http.HttpCacheControl",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpDate",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpCaseTokenList",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpInteger",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpContentRange",
	    "w3c.www.http.HttpMimeType",
	    "w3c.www.http.HttpEntityTag",
	    "w3c.www.http.HttpDate",
	    "w3c.www.http.HttpDate",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpInteger",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpChallenge",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpDate",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpWarningList",
	    "w3c.www.http.HttpChallenge",
	    "w3c.tools.store.Resource",
	    "java.lang.Object",
	    "w3c.tools.store.ResourceStore",
	    "w3c.jigsaw.auth.AuthRealm",
	    "w3c.jigsaw.http.httpd",
	    "w3c.jigsaw.resources.ResourceFilter",
	    "w3c.jigsaw.resources.FilteredResource",
	    "w3c.jigsaw.auth.AuthUser",
	    "w3c.jigsaw.auth.AuthFilter",
	    "w3c.jigsaw.resources.HTTPResource",
	    "w3c.tools.store.Resource",
	    "w3c.jigsaw.http.httpd",
	    "w3c.jigsaw.config.PropertySet",
	    "w3c.www.http.HttpAcceptList",
	    "w3c.www.http.HttpAcceptCharsetList",
	    "w3c.www.http.HttpTokenList",
	    "w3c.www.http.HttpAcceptLanguageList",
	    "w3c.www.http.HttpCredential",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpDate",
	    "w3c.www.http.HttpEntityTagList",
	    "w3c.www.http.HttpEntityTagList",
	    "w3c.www.http.HttpDate",
	    "w3c.www.http.HttpInteger",
	    "w3c.www.http.HttpCredential",
	    "w3c.www.http.HttpRangeList",
	    "w3c.www.http.HttpString",
	    "w3c.www.http.HttpString",
	    "w3c.jigsaw.http.GeneralProp",
	    "w3c.jigsaw.http.LoggingProp",
	    "w3c.jigsaw.indexer.Directory",
	    "w3c.jigsaw.resources.NegotiatedResource"
	};

	try
	{
	    Object handler;

	    for (int i = 0; i < classes.length; ++i) {
		try {
			Class.forName(classes[i]).newInstance();
		} catch (Exception e) {
			System.out.println("Exception ("+e.toString()+") while trying to create an instance of: "+classes[i]);
		}
	    }
	    URL url = new URL("http://www.schani.net/");

	    System.out.println("url " + url.toString());

	    handler = Class.forName("sun.net.www.protocol.http.Handler").newInstance();

	    URLStreamHandler h;

	    System.out.println("before");
	    h = (java.net.URLStreamHandler)handler;
	    System.out.println("after");

	    handler = Class.forName("sun.net.www.protocol.http.Handler").newInstance();

	    System.out.println("before");
	    h = (java.net.URLStreamHandler)handler;
	    System.out.println("after");
	}
	catch (Exception exc)
	{
	}
    }
}
