---
language: en
layout: default
category: Documentation
title: Userspace Clients General Usage
---

[Documentation](documentation.html) > [Userspace Clients](documentation.html#userspace-clients) > General usage

# Userspace Clients General Usage

## Index

1. [Syntax](#syntax)
2. [Arguments](#arguments)
	1. [`(jool | jool_siit)`](#jool--jool_siit)
	2. [`<argp1>`](#argp1)
	4. [`<mode>`](#mode)
	5. [`<operation>`](#operation)
	6. [`<argp2>`](#argp2)
3. [Quirks](#quirks)

## Syntax

All userspace client command line requests adhere to the following syntax:

	(jool | jool_siit) [<argp1>] <mode> <operation> [<argp2>]

## Arguments

### `(jool | jool_siit)`

`jool` and `jool_siit` are the names of the two available userspace client binaries. The `jool` client speaks to the `jool` kernel module, and the `jool_siit` client speaks to the `jool_siit` kernel module.

### `<argp1>`

`argp1` is a first batch of classic [`getopt`](http://man7.org/linux/man-pages/man3/getopt.3.html)-based arguments:

```bash
<argp1> := ( --help | --usage | --version | --instance <name> | --file <file> )
```

- `--help` (`-?`): Print a summary of this document.
- `--usage`: Print program usage syntax. (Also available amidst the output of `--help`.)
- `--version` (`-V`): Print program version.
- `--instance <name>` (`-i <name>`): Specify the name of the Jool instance to interact with.
- `--file <file>` (`-f <name>`): Specify a JSON file which contains the name of the instance to interact with.

The instance name is a 15-character maximum ASCII string. It defaults to `default`. It's the same one you specify during [`instance add`](usr-flags-instance.html).

The bare minimum legal JSON file looks like this:

	{
		"instance": "<name>"
	}

All JSON tags other than `"instance"` are ignored. (The idea is to allow you to reuse an [atomic configuration](config-atomic.html) file as a `--file` file.)

### `<mode>`

`<mode>` is (usually) one of the following keywords:

- [`instance`](usr-flags-instance.html)
- [`stats`](usr-flags-stats.html)
- [`global`](usr-flags-global.html)
- [`eamt`](usr-flags-eamt.html) (SIIT only)
- [`denylist4`](usr-flags-denylist4.html) (SIIT only)
- [`pool4`](usr-flags-pool4.html) (NAT64 only)
- [`bib`](usr-flags-bib.html) (NAT64 only)
- [`session`](usr-flags-session.html) (NAT64 only)

### `<operation>`

`<operation>` is (usually) one of the following keywords:

- `display`
- `add`
- `update`
- `remove`
- `flush`

See the respective mode documentation for details.

### `<argp2>`

`<arg2>` is a second batch of traditional getopt-parsed arguments. (Though these are actually handled by [argp](https://www.gnu.org/software/libc/manual/html_node/Argp.html).) They depend on the `<mode> <operation>` context. For example, list the `instance add` flags by running

{% highlight bash %}
user@T:~$ jool instance add --help
{% endhighlight %}

The only exception is [`global update`](usr-flags-global.html), where the value key acts as a third keyword level:

{% highlight bash %}
user@T:~$ jool global update <key> --help
{% endhighlight %}

## Quirks

As long as you don't reach ambiguity, you can abbreviate keywords:

{% highlight bash %}
user@T:~# jool_siit i a    # instance add
user@T:~# jool      g u    # global update
user@T:~# jool_siit s d    # stats display
user@T:~# jool      s d    # Error: stats or session? display
{% endhighlight %}

Of course, do not rely on these shorthands during scripts. There is no guarantee that future new keywords will not induce ambiguity.
