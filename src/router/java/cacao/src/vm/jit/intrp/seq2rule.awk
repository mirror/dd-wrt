BEGIN {
  FS="\t";
}
{
  name = $4;
  gsub(/ /,"_",name);
  print name" = "$4;
}
