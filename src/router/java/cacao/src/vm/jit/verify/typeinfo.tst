# Input for testing the typeinfo_ functions
#
# simple merges (result is common ancestor)
#
()Ljava/lang/Object;		()Ljava/lang/Object;		()Ljava/lang/Object;
()Ljava/lang/Object;		()Ljava/lang/String;		()Ljava/lang/Object;
()Ljava/lang/Number;		()Ljava/lang/Integer;		()Ljava/lang/Number;
()Ljava/lang/Object;		()Ljava/lang/Integer;		()Ljava/lang/Object;
()Ljava/lang/Object;		()Ljava/lang/Cloneable;		()Ljava/lang/Object;
()Ljava/lang/Cloneable;		()Ljava/lang/Cloneable;		()Ljava/lang/Cloneable;
()Ljava/lang/Cloneable;		()[Ljava/lang/Object;		()Ljava/lang/Cloneable;
()Ljava/lang/Cloneable;		()[I				()Ljava/lang/Cloneable;
()Ljava/io/Serializable;	()[Ljava/lang/Object;		()Ljava/io/Serializable;
()Ljava/io/Serializable;	()[D				()Ljava/io/Serializable;
()Ljava/lang/Object;		()[Ljava/lang/Object;		()Ljava/lang/Object;
()Ljava/lang/Object;		()[I				()Ljava/lang/Object;
()L$ARRAYSTUB$;			()L$ARRAYSTUB$;			()L$ARRAYSTUB$;
()L$ARRAYSTUB$;          	()Ljava/lang/Object;		()Ljava/lang/Object;
()L$ARRAYSTUB$;          	()Ljava/lang/Cloneable;		()Ljava/lang/Cloneable;
()L$ARRAYSTUB$;          	()Ljava/io/Serializable;	()Ljava/io/Serializable;
()L$NULL$;			()L$NULL$;			()L$NULL$;
()L$NULL$;			()[Ljava/lang/Object;		()[Ljava/lang/Object;
#
#
# merges where the result is a mergedlist
#
()Ljava/lang/String;	()Ljava/lang/Number;	(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;
#
#
# merges with one mergedlist where the resulting mergedlist is unchanged
#
(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;	()Ljava/lang/String;	(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;
(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;	()Ljava/lang/Number;	(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		()Ljava/lang/Integer;	(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		()Ljava/lang/Long;	(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		()L$NULL$;		(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;
#
#
# merges with one mergedlist where the result has no mergedlist
#
(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;	()Ljava/lang/Object;	()Ljava/lang/Object;
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		()Ljava/lang/Number;	()Ljava/lang/Number;
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		()Ljava/lang/Object;	()Ljava/lang/Object;
#
#
# one mergedlist where the resulting mergeslist is longer
#
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		()Ljava/lang/Double;	(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Double;)Ljava/lang/Number;
#
#
# merges with two mergedlists with the same typeclass
#
(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;	(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;	(Ljava/lang/String;Ljava/lang/Number;)Ljava/lang/Object;
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Float;)Ljava/lang/Number;	(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/Float;)Ljava/lang/Number;
#
#
# merges with two mergedlists with different typeclass
#
(Ljava/lang/Integer;Ljava/lang/Long;)Ljava/lang/Number;		(Ljava/lang/Integer;Ljava/lang/String;)Ljava/lang/Object;	(Ljava/lang/Integer;Ljava/lang/Long;Ljava/lang/String;)Ljava/lang/Object;
#
#
# merging primitive arrays
#
()[I	()[I	()[I
()[I	()[F	()L$ARRAYSTUB$;
#
#
# merging reference arrays with primitive arrays
#
()[I	()[Ljava/lang/String;	()L$ARRAYSTUB$;
