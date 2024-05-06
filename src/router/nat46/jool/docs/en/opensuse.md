---
language: en
layout: default
category: Documentation
title: openSUSE
---

[Documentation](documentation.html) > [Installation](documentation.html#installation) > openSUSE

# Jool in openSUSE

We're pleased to have a volunteer for package maintenance for Jool in the openSUSE distribution. As with the OpenWRT releases, please note that these binaries are not maintained nor supervised by the Jool team, but we are still available for advice if issues arise.

It might take an indeterminate amount of time for the latest version of Jool to be uploaded to this repository. Remember that you can find previous versions of this site's documentation in the [download page](download.html).

## Installation

> ![Warning!](../images/warning.svg) If you have somehow previously installed Jool from source in your machine, then those binaries may conflict with the ones installed here.
>
> You may uninstall source-installed binaries by following [these steps](install.html#uninstalling).

Courtesy of Martin Hauke:

{% highlight bash %}
$ sudo zypper addrepo -f obs://home:mnhauke:jool jool
$ sudo zypper install jool-kmp-default jool-tools
{% endhighlight %}

That's it!

Here's a quick link back to the [basic tutorials list](documentation.html#basic-tutorials).
