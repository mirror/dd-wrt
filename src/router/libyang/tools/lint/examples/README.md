# YANGLINT - Interactive Mode Examples

This text provides several use-case of the `yanglint(1)` interactive
mode. For basic information about the `yanglint(1)` usage, please see
the man page.

The examples are supposed to be went through one by one. Some of the examples
suppose the specific schemas loaded in some of the previous example is still
loaded. If an addition work is need, the *preparation* part in the example
provides information what to do.

To show all available command of the `yanglint(1)`, use the `help` command:
```
> help
Available commands:
  help            Display commands description
  add             Add a new model
  print           Print model
  data            Load, validate and optionally print instance data
  xpath           Get data nodes satisfying an XPath expression
  list            List all the loaded models
  feature         Print/enable/disable all/specific features of models
  searchpath      Set the search path for models
  clear           Clear the context - remove all the loaded models
  verb            Change verbosity
  quit            Quit the program
  ?               Display commands description
  exit            Quit the program
```
To show the information about the specific command, use the `help` command in
combination with the command name you are interested in:
```
> help searchpath
searchpath <model-dir-path>
```

The input files referred in this document are available together with this
document.

## Duplicit Data Model

Let's have two data models [module1.yang](./module1.yang)
and [module1b.yang](./module1b.yang).
They differ in the module name but their namespaces are the same.

Preparation:

```
> clear
> add module1.yang
> list
```

Output:

```
List of the loaded models (mod-set-id 5):
        ietf-inet-types@2013-07-15
        ietf-yang-types@2013-07-15
        ietf-yang-library@2015-07-03
        module1
```

Command and its output:

```
> add module1b.yang
libyang[0]: Two different modules ("module1" and "module1b") have the same namespace "urn:yanglint:module".
libyang[0]: Module "module1b" parsing failed.
```

## Yang Data Model Validation

**YANG/YIN syntax**

`module2.yin` contains a syntax error.
There is a bad syntax of the `type` statement in YIN file.

```
<type value="string"/>
```

instead of

```
<type name="string"/>
```

Preparation:

```
> clear
```

Command and its output:

```
> add module2.yin
libyang[0]: Missing argument "name" to keyword "type".
libyang[0]: Module "module1" parsing failed.
```

Similarly, there is a typo in `module2.yang`.

**XPath errors**

`libyang` and `yanglint(1)` is able to detect also errors in XPath expressions.
In `module3.yang` the `must` expression refers to the node which does not exists.

Preparation:

```
> clear
```

Command and its output:

```
> add module3.yang
libyang[0]: Schema node "a" not found (../c/a).
libyang[0]: Path is related to the previous error message. (path: /module3:m)
libyang[0]: Module "module3" parsing failed.
```

Note that libyang does not provide line numbers of the error. Instead it tries to
print the path to the related node. in some cases (as this one) it is not able
to print the path immediately so the path (to the node `m` which refers node which
does not exist) is printed in the second message.

## Data Validation

Preparation:

```
> clear
> add ietf-netconf-acm.yang
```

**Unknown data**

By default, yanglint ignores unknown data and no error is printed (you can
compare real content of the `datastore.xml` file and what yanglint prints
in the following command if you add `-f xml` option).

Command and its output:

```
> data -t config datastore.xml
```

We use option `-t` to specify type of the data in `datastore.xml`. By the
`config` value we declare that the input file contains all the configuration
data (with at least all the mandatory nodes as required by the loaded schemas),
but without the status data. More examples of different data types will follow.

To handle unknown data as error, use strict mode (`-s` option).

Command and its output:

```
> data -t config -s datastore.xml
libyang[0]: Unknown element "interfaces". (path: /)
Failed to parse data.
```

Note that in case of working with complete datastore including the status data
(no `-t` option is specified), `yanglint(1)` has to add status data from its
internal `ietf-yang-library` module. Using the `-s` option in this case forces
validation in time of parsing the input file so it is expected to include also
the mandatory status data from the `ietf-yang-library` module.

**RPC and RPC-reply**

It is possible to validate RPCs and their replies as well.

Peparation:

```
> clear
> add module4.yang
```

Command and its output:

```
> data -t rpc rpc.xml
```

Reply to this RPC can be validated too, but it must be specified, to which
RPC it is a reply to, because it is not included in the reply itself.

Command and its output:

```
> data -t rpcreply rpc-reply.xml rpc.xml
```

**action and action-reply**

Actions are validated the same way as RPCs except you need to be careful
about the input file structure.

Preparation

```
> clear
> add module4.yang
```

Command and its output:

```
> data -t rpc action.xml
```

Command and its output:

```
> data -t rpc action-reply.xml action.xml
```

**notification**

Both top-level and nested notification can be validated.

Preparation

```
> clear
> add module4.yang
```

Command and its output:

```
> data -t notif notification.xml
```

Command and its output:

```
> data -t notif nested-notification.xml
```


**Multiple top-level elements in a single document**

As a feature and in conflict with the XML definition, `yanglint(1)` (and libyang)
is able to read XML files with multiple top-level elements. Such documents
are not well-formed according to the XML spec, but it fits to how the YANG
interconnects data trees (defined as top-level elements of a single schema
or by multiple schemas).

Preparation:

```
> clear
> add ietf-netconf-acm.yang
> add ietf-interfaces.yang
> add ietf-ip.yang
> add iana-if-type.yang
```

Command and its output:

```
> data -t config -s datastore.xml
```

**Different data content types**

Since NETCONF requires the data described by YANG to be used in different
situations (e.g. as <edit-config data>, result of the <get> with status data
included or as a result of the <get-config> without the status data and
possibly filtered, so without specified subtrees), it must be possible to
specify which kind of data is going to be parsed. In `yanglint(1)`, this is done
via `-t` option. The list of supported modes can be displayed by the `-h`
option given to the `data` command. In general, the `auto` value lets the
`yanglint(1)` to recognize the data type automatically by the additional top-level
elements added to the parsed data. This is the same way as `pyang(1)` uses. Note,
that the automatic data type recognition is available only for the XML input.

**Malformed XML data**

Command and its output:

```
> data -t edit config-missing-key.xml
libyang[0]: Invalid (mixed names) opening (nam) and closing (name) element tags. (path: /nacm/groups/group/nam)
Failed to parse data.
```

**State information in edit-config XML**

Command and its output:

```
> data -t edit config-unknown-element.xml
libyang[0]: Unknown element "denied-operations". (path: /ietf-netconf-acm:nacm/denied-operations)
Failed to parse data.
```

**Missing required element in NETCONF data**

Command and its output:

```
> data data-missing-key.xml
libyang[0]: Missing required element "name" in "rule". (path: /ietf-netconf-acm:nacm/rule-list[name='almighty']/rule)
Failed to parse data.
```

**Malformed XML**

Command and its output:

```
> data data-malformed-xml.xml
libyang[0]: Invalid (mixed names) opening (nam) and closing (rule) element tags. (path: /nacm/rule-list/rule/nam)
Failed to parse data.
```

Command and its output:

```
> data data-malformed-xml2.xml
libyang[0]: Invalid (mixed names) opening (module-name) and closing (name) element tags. (path: /nacm/rule-list/rule/name/module-name)
Failed to parse data.
```

**Bad value**

Command and its output:

```
> data data-out-of-range-value.xml
libyang[0]: Invalid value "-1" in "denied-operations" element. (path: /ietf-netconf-acm:nacm/denied-operations)
Failed to parse data.
```

## Validation of "when" Statement in Data

Preparation:

```
> clear
> add ietf-netconf-acm-when.yang
```

**`When` condition is not satisfied since `denied-operation = 0`**

Command and its output:

```
> data data-acm.xml
```

The command succeeds. It is because `yanglint(1)` (via `libyang`) performs
autodeletion - the not satisfied `when` condition in `denied-data-writes`
causes its automatic (silent) deletion.

## Printing a Data Model

Preparation:

```
> clear
> add ietf-netconf-acm.yang
```

**Print a `pyang`-style tree**

Command and its output:

```
> print ietf-netconf-acm
module: ietf-netconf-acm
   +--rw nacm
      +--rw enable-nacm?              boolean <true>
      +--rw read-default?             action-type <permit>
      +--rw write-default?            action-type <deny>
      +--rw exec-default?             action-type <permit>
      +--rw enable-external-groups?   boolean <true>
      +--ro denied-operations         ietf-yang-types:zero-based-counter32
      +--ro denied-data-writes        ietf-yang-types:zero-based-counter32
      +--ro denied-notifications      ietf-yang-types:zero-based-counter32
      +--rw groups
      |  +--rw group* [name]
      |     +--rw name         group-name-type
      |     +--rw user-name*   user-name-type
      +--rw rule-list* [name]
         +--rw name     string
         +--rw group*   union
         +--rw rule* [name]
            +--rw name                 string
            +--rw module-name?         union <*>
            +--rw (rule-type)?
            |  +--:(protocol-operation)
            |  |  +--rw rpc-name?             union
            |  +--:(notification)
            |  |  +--rw notification-name?    union
            |  +--:(data-node)
            |     +--rw path                  node-instance-identifier
            +--rw access-operations?   union <*>
            +--rw action               action-type
            +--rw comment?             string
>
```

**Obtain information about model**

Command and its output:

```
> print -f info ietf-netconf-acm
Module:    ietf-netconf-acm
Namespace: urn:ietf:params:xml:ns:yang:ietf-netconf-acm
Prefix:    nacm
Desc:      NETCONF Access Control Model.

           Copyright (c) 2012 IETF Trust and the persons identified as
           authors of the code.  All rights reserved.

           Redistribution and use in source and binary forms, with or
           without modification, is permitted pursuant to, and subject
           to the license terms contained in, the Simplified BSD
           License set forth in Section 4.c of the IETF Trust's
           Legal Provisions Relating to IETF Documents
           (http://trustee.ietf.org/license-info).

           This version of this YANG module is part of RFC 6536; see
           the RFC itself for full legal notices.
Reference:
Org:       IETF NETCONF (Network Configuration) Working Group
Contact:   WG Web:   <http://tools.ietf.org/wg/netconf/>
           WG List:  <mailto:netconf@ietf.org>

           WG Chair: Mehmet Ersue
                     <mailto:mehmet.ersue@nsn.com>

           WG Chair: Bert Wijnen
                     <mailto:bertietf@bwijnen.net>

           Editor:   Andy Bierman
                     <mailto:andy@yumaworks.com>

           Editor:   Martin Bjorklund
                     <mailto:mbj@tail-f.com>
YANG ver:  1.0
Deviated:  no
Implement: yes
URI:
Revisions: 2012-02-22
Includes:
Imports:   yang:ietf-yang-types
Typedefs:  user-name-type
           matchall-string-type
           access-operations-type
           group-name-type
           action-type
           node-instance-identifier
Idents:
Features:
Augments:
Deviation:
Data:      container "nacm"
```

**Print information about specific model part**

Print information about a node:

```
> print -f info -P /ietf-netconf-acm:nacm/denied-operations ietf-netconf-acm
Leaf:      denied-operations
Module:    ietf-netconf-acm
Desc:      Number of times since the server last restarted that a
           protocol operation request was denied.
Reference:
Config:    read-only
Status:    current
Mandatory: yes
Type:      zero-based-counter32
Units:
Default:
If-feats:
When:
Must:
```
Print detailed information about its type `zero-based-counter32`:
```
> print -f info -P type/ietf-netconf-acm:nacm/denied-operations ietf-netconf-acm
Base type: uint32
Range:
Superior:  ietf-yang-types:zero-based-counter32
```
Print information about the typedef `zero-based-counter32` it was derived from:
```
> print -f info -P typedef/zero-based-counter32 ietf-yang-types
Typedef:   zero-based-counter32
Module:    ietf-yang-types
Desc:      The zero-based-counter32 type represents a counter32
           that has the defined 'initial' value zero.

           A schema node of this type will be set to zero (0) on creation
           and will thereafter increase monotonically until it reaches
           a maximum value of 2^32-1 (4294967295 decimal), when it
           wraps around and starts increasing again from zero.

           Provided that an application discovers a new schema node
           of this type within the minimum time to wrap, it can use the
           'initial' value as a delta.  It is important for a management
           station to be aware of this minimum time and the actual time
           between polls, and to discard data if the actual time is too
           long or there is no defined minimum time.

           In the value set and its semantics, this type is equivalent
           to the ZeroBasedCounter32 textual convention of the SMIv2.
Reference: RFC 4502: Remote Network Monitoring Management Information
                     Base Version 2
Status:    current
Base type: uint32
Range:
Superior:  counter32
Units:
Default:   0
```
Finally, print information about the typedef `counter32` the other typedef `zero-based-counter32` was derived from:
```
> print -f info -P typedef/counter32 ietf-yang-types
Typedef:   counter32
Module:    ietf-yang-types
Desc:      The counter32 type represents a non-negative integer
           that monotonically increases until it reaches a
           maximum value of 2^32-1 (4294967295 decimal), when it
           wraps around and starts increasing again from zero.

           Counters have no defined 'initial' value, and thus, a
           single value of a counter has (in general) no information
           content.  Discontinuities in the monotonically increasing
           value normally occur at re-initialization of the
           management system, and at other times as specified in the
           description of a schema node using this type.  If such
           other times can occur, for example, the creation of
           a schema node of type counter32 at times other than
           re-initialization, then a corresponding schema node
           should be defined, with an appropriate type, to indicate
           the last discontinuity.

           The counter32 type should not be used for configuration
           schema nodes.  A default statement SHOULD NOT be used in
           combination with the type counter32.

           In the value set and its semantics, this type is equivalent
           to the Counter32 type of the SMIv2.
Reference: RFC 2578: Structure of Management Information Version 2
                     (SMIv2)
Status:    current
Base type: uint32
Range:
Superior:  uint32
Units:
Default:
```

## Query using NETCONF data

Preparation:

```
> clear
> add ietf-netconf-acm.yang
```

**Print all `user-name` elements that occure in data**

Command and its output:

```
> xpath -e //ietf-netconf-acm:user-name data-acm.xml
Result:
        Leaflist "user-name" (val: smith)
        Leaflist "user-name" (val: smith)
        Leaflist "user-name" (val: doe)

```

**Print all data that satisfies condition**

Command and its output:

```
> xpath -e //ietf-netconf-acm:user-name[text()="smith"] data-acm.xml
Result:
        Leaflist "user-name" (val: smith)
        Leaflist "user-name" (val: smith)

```

## Usage of `feature` in Yang

Preparation:

```
> clear
> add ietf-interfaces.yang
> add ietf-ip.yang
> add iana-if-type.yang
```

Note: This example also shows `JSON` output of the command.

Command and its output:
```
> feature -e * ietf-ip
> data -f json -t config data-ip.xml
{
  "ietf-interfaces:interfaces": {
    "interface": [
      {
        "name": "eth0",
        "description": "Wire Connection",
        "type": "iana-if-type:ethernetCsmacd",
        "enabled": true,
        "ietf-ip:ipv4": {
          "address": [
            {
              "ip": "192.168.1.15",
              "netmask": "255.255.255.0"
            },
            {
              "ip": "192.168.1.10",
              "netmask": "255.255.255.0"
            }
          ]
        }
      }
    ]
  }
}
```

