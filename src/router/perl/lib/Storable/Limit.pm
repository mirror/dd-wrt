# bisected by stacksize
$Storable::recursion_limit = 500
  unless defined $Storable::recursion_limit;
$Storable::recursion_limit_hash = 265
  unless defined $Storable::recursion_limit_hash;
1;
