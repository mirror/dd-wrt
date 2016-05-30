public class testEx
{
  public static void main(String[] a)
  {
    try
    {
	NullPointerException e =  new NullPointerException( "OK" );
	e.getClass();
	throw e;
    }
    catch ( NullPointerException e )
    {
	System.out.println( e.getMessage() );
    }
  }
}
