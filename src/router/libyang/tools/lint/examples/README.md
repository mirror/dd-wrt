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
  add             Add a new module from a specific file
  load            Load a new schema from the searchdirs
  print           Print a module
  data            Load, validate and optionally print instance data
  list            List all the loaded modules
  feature         Print all features of module(s) with their state
  searchpath      Print/set the search path(s) for schemas
  clear           Clear the context - remove all the loaded modules
  verb            Change verbosity
  debug           Display specific debug message groups
  quit            Quit the program
  ?               Display commands description
  exit            Quit the program
```
To show the information about the specific command, use the `help` command in
combination with the command name you are interested in:
```
> help searchpath
Usage: searchpath [--clear] [<modules-dir-path> ...]
                  Set paths of directories where to search for imports and includes
                  of the schema modules. Subdirectories are also searched. The current
                  working directory and the path of the module being added is used implicitly.
                  The 'load' command uses these paths to search even for the schema modules
                  to be loaded.
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
List of the loaded models:
    i ietf-yang-metadata@2016-08-05
    I yang@2022-06-16
    i ietf-inet-types@2013-07-15
    i ietf-yang-types@2013-07-15
    I ietf-yang-schema-mount@2019-01-14
    I module1
```

Command and its output:

```
> add module1b.yang
libyang[0]: Two different modules ("module1" and "module1b") have the same namespace "urn:yanglint:module".
libyang[0]: Parsing module "module1b" failed.
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
libyang[0]: Unexpected attribute "value" of "type" element. (path: Line number 8.)
libyang[0]: Parsing module "module2" failed.
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
libyang[1]: Schema node "a" for parent "/module3:c" not found; in expr "../c/a" with context node "/module3:m".
```

Note that libyang prints only a warning in this case because it is not
specified that XPath expressions must refer to existing nodes.

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

Command and its output:

```
> data -t config datastore.xml
libyang[0]: No module with namespace "urn:ietf:params:xml:ns:yang:ietf-interfaces" in the context. (path: Line number 20.)
YANGLINT[E]: Failed to parse input data file "datastore.xml".
```

Note that in case of working with complete datastore including the status data
(no `-t` option is specified), `yanglint(1)` has to add status data from its
internal `ietf-yang-library` module.

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

Reply to this RPC can be validated too, but it must be nested in the original
RPC element.

Command and its output:

```
> data -t reply ../tools/lint/examples/rpc-reply.xml
```

**action and action-reply**

Actions are validated the same way as RPCs except you need to be careful
about the input file structure. No NETCONF-specific envelopes are expected.

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
> data -t config datastore.xml
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
libyang[0]: Node "nam" not found as a child of "group" node. (path: Schema location "/ietf-netconf-acm:nacm/groups/group", data location "/ietf-netconf-acm:group", line number 19.)
YANGLINT[E]: Failed to parse input data file "config-missing-key.xml".
```

**State information in edit-config XML**

Command and its output:

```
> data -t edit config-unknown-element.xml
libyang[0]: Unexpected data state node "denied-operations" found. (path: Schema location "/ietf-netconf-acm:nacm/denied-operations", data location "/ietf-netconf-acm:nacm", line number 24.)
YANGLINT[E]: Failed to parse input data file "config-unknown-element.xml".
```

**Missing required element in NETCONF data**

Command and its output:

```
> data data-missing-key.xml
libyang[0]: List instance is missing its key "name". (path: Schema location "/ietf-netconf-acm:nacm/rule-list/rule", data location "/ietf-netconf-acm:rule", line number 10.)
YANGLINT[E]: Failed to parse input data file "data-missing-key.xml".
```

**Malformed XML**

Command and its output:

```
> data data-malformed-xml.xml
libyang[0]: Node "nam" not found as a child of "rule" node. (path: Schema location "/ietf-netconf-acm:nacm/rule-list/rule", data location "/ietf-netconf-acm:rule", line number 8.)
YANGLINT[E]: Failed to parse input data file "data-malformed-xml.xml".
```

Command and its output:

```
> data data-malformed-xml2.xml
libyang[0]: Child element "module-name" inside a terminal node "name" found. (path: Schema location "/ietf-netconf-acm:nacm/rule-list/rule/name", data location "/ietf-netconf-acm:name", line number 7.)
YANGLINT[E]: Failed to parse input data file "data-malformed-xml2.xml".
```

**Bad value**

Command and its output:

```
> data data-out-of-range-value.xml
libyang[0]: Value "-1" is out of type uint32 min/max bounds. (path: Schema location "/ietf-netconf-acm:nacm/denied-operations", data location "/ietf-netconf-acm:nacm", line number 24.)
YANGLINT[E]: Failed to parse input data file "data-out-of-range-value.xml".
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
libyang[0]: When condition "../denied-operations > 0" not satisfied. (path: Schema location "/ietf-netconf-acm-when:nacm/denied-data-writes", data location "/ietf-netconf-acm-when:nacm/denied-data-writes".)
YANGLINT[E]: Failed to parse input data file "data-acm.xml".
```

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
     +--rw enable-nacm?              boolean
     +--rw read-default?             action-type
     +--rw write-default?            action-type
     +--rw exec-default?             action-type
     +--rw enable-external-groups?   boolean
     +--ro denied-operations         yang:zero-based-counter32
     +--ro denied-data-writes        yang:zero-based-counter32
     +--ro denied-notifications      yang:zero-based-counter32
     +--rw groups
     |  +--rw group* [name]
     |     +--rw name         group-name-type
     |     +--rw user-name*   user-name-type
     +--rw rule-list* [name]
        +--rw name     string
        +--rw group*   union
        +--rw rule* [name]
           +--rw name                 string
           +--rw module-name?         union
           +--rw (rule-type)?
           |  +--:(protocol-operation)
           |  |  +--rw rpc-name?   union
           |  +--:(notification)
           |  |  +--rw notification-name?   union
           |  +--:(data-node)
           |     +--rw path    node-instance-identifier
           +--rw access-operations?   union
           +--rw action               action-type
           +--rw comment?             string
```

**Print information about specific model part**

Command and its output:

```
> print -f info -P /ietf-netconf-acm:nacm/ietf-netconf-acm:enable-nacm ietf-netconf-acm
leaf enable-nacm {
  ietf-netconf-acm:default-deny-all;
  type boolean;
  default "true";
  config true;
  status current;
  description
    "Enables or disables all NETCONF access control
     enforcement.  If 'true', then enforcement
     is enabled.  If 'false', then enforcement
     is disabled.";
}
```

## Usage of `feature` in Yang

Preparation:

```
> clear
> add ietf-interfaces.yang
> add ietf-ip.yang -F ietf-ip:*
> add iana-if-type.yang
```

Note: This example also shows `JSON` output of the command.

Command and its output:
```
> feature ietf-ip
ietf-ip features:
        ipv4-non-contiguous-netmasks (on)
        ipv6-privacy-autoconf        (on)
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

## YANG modules with the Schema Mount extension

In these examples the non-interactive `yanglint` is used to simplify creating the context, a `yang-library` data file is
used. The working directory is `libyang/tools/lint/examples` and *libyang* must be installed.

**Print tree output of a model with Schema Mount**

Command and its output:

```
$ yanglint -f tree -p . -Y sm-context-main.xml -x sm-context-extension.xml sm-main.yang
module: sm-main
  +--mp root* [node]
  |  +--rw node    string
  +--mp root2
  +--rw root3
     +--mp my-list* [name]
        +--rw things/* [name]
        |  +--rw name         -> /if:interfaces/if:interface/if:name
        |  +--rw attribute?   uint32
        +--rw not-compiled/
        |  +--rw first?    string
        |  +--rw second?   string
        +--rw interfaces@
        |  +--rw interface* [name]
        |     +--rw name    string
        |     +--rw type    identityref
        +--rw name    string
```

**Validating and printing mounted data**

Command and its output:

```
$ yanglint -f json -t config -p . -Y sm-context-main.xml -x sm-context-extension.xml sm-data.xml
{
  "ietf-interfaces:interfaces": {
    "interface": [
      {
        "name": "eth0",
        "type": "iana-if-type:ethernetCsmacd"
      },
      {
        "name": "eth1",
        "type": "iana-if-type:ethernetCsmacd"
      }
    ]
  },
  "sm-main:root3": {
    "my-list": [
      {
        "name": "list item 1",
        "sm-extension:things": [
          {
            "name": "eth0",
            "attribute": 1
          }
        ]
      }
    ]
  }
}
```
