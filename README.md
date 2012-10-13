This is a standalone [envdir]-like utility -- it lets you read the first
lines of files in a directory into corresponding environment variables.

For example, if you run

    denv confdir progname --other-args

where confdir is a directory containing these files

    $ cat confdir/FOO
    peanut butter
    $ cat confdir/BAR
    sriracha
    $ cat confdir/BAZ     # where BAZ is executable
    #!/bin/sh
    echo "garlic"

then progname will be run with an environment like

    env FOO="peanut butter" BAR="sriracha" BAZ="garlic" progname --other-args

If a file exists but is empty, then the variable will be cleared.
Executable files yield their first line of output.

(If you find this useful, you might also want to check out [skel].)

While I highly recommend daemontools (or [runit], etc.), there's no
reason this handy little tool should depend on it.

[envdir]: http://cr.yp.to/daemontools/envdir.html
[skel]: https://github.com/silentbicycle/skel
[runit]: http://smarden.org/runit/
