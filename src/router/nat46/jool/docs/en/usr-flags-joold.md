---
language: en
layout: default
category: Documentation
title: --joold
---

[Documentation](documentation.html) > [Userspace Application Arguments](documentation.html#userspace-application-arguments) > joold

# joold

## Index

1. [Description](#description)
2. [Syntax](#syntax)
3. [Arguments](#arguments)
   1. [Operations](#operations)

## Description

Commands the kernel module's [SS submodule](session-synchronization.html).

Rather confusingly, this is not what you use to _configure_ SS; that's [`global`](usr-flags-global.html)'s responsibility. `joold` can only request specific actions.

## Syntax

	jool joold (
		advertise
	)

## Arguments

### Operations

* `advertise`: Commands the module to multicast the entire session database. This can be useful if you've recently added a new NAT64 to the cluster.  
_The size of the session database can make this is an expensive operation_; executing this command repeatedly is not recommended.  
Only one Jool instance needs to advertise when a new NAT64 joins the group; the databases are supposed to be identical.  
This exists because the synchronization protocol, at least in this first iteration, is very minimalistic. The instances only announce their sessions to everyone else; there are no handshakes or agreements. Full advertisements need to be triggered manually.

## Examples

Multicast the entire session database:

	$ jool joold advertise

