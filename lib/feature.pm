# -*- buffer-read-only: t -*-
# !!!!!!!   DO NOT EDIT THIS FILE   !!!!!!!
# This file is built by regen/feature.pl.
# Any changes made here will be lost!

package feature;

our $VERSION = '1.35';

our %feature = (
    fc              => 'feature_fc',
    say             => 'feature_say',
    state           => 'feature_state',
    switch          => 'feature_switch',
    evalbytes       => 'feature_evalbytes',
    postderef       => 'feature_postderef',
    array_base      => 'feature_arybase',
    signatures      => 'feature_signatures',
    current_sub     => 'feature___SUB__',
    lexical_subs    => 'feature_lexsubs',
    postderef_qq    => 'feature_postderef_qq',
    unicode_eval    => 'feature_unieval',
    unicode_strings => 'feature_unicode',
);

our %feature_bundle = (
    "5.10"    => [qw(array_base say state switch)],
    "5.11"    => [qw(array_base say state switch unicode_strings)],
    "5.15"    => [qw(current_sub evalbytes fc say state switch unicode_eval unicode_strings)],
    "all"     => [qw(array_base current_sub evalbytes fc lexical_subs postderef postderef_qq say signatures state switch unicode_eval unicode_strings)],
    "default" => [qw(array_base)],
);

$feature_bundle{"5.12"} = $feature_bundle{"5.11"};
$feature_bundle{"5.13"} = $feature_bundle{"5.11"};
$feature_bundle{"5.14"} = $feature_bundle{"5.11"};
$feature_bundle{"5.16"} = $feature_bundle{"5.15"};
$feature_bundle{"5.17"} = $feature_bundle{"5.15"};
$feature_bundle{"5.18"} = $feature_bundle{"5.15"};
$feature_bundle{"5.19"} = $feature_bundle{"5.15"};
$feature_bundle{"5.20"} = $feature_bundle{"5.15"};
$feature_bundle{"5.9.5"} = $feature_bundle{"5.10"};

our $hint_shift   = 26;
our $hint_mask    = 0x1c000000;
our @hint_bundles = qw( default 5.10 5.11 5.15 );

# This gets set (for now) in $^H as well as in %^H,
# for runtime speed of the uc/lc/ucfirst/lcfirst functions.
# See HINT_UNI_8_BIT in perl.h.
our $hint_uni8bit = 0x00000800;

# TODO:
# - think about versioned features (use feature switch => 2)

=head1 NAME

feature - Perl pragma to enable new features

=head1 SYNOPSIS

    use feature qw(say switch);
    given ($foo) {
        when (1)          { say "\$foo == 1" }
        when ([2,3])      { say "\$foo == 2 || \$foo == 3" }
        when (/^a[bc]d$/) { say "\$foo eq 'abd' || \$foo eq 'acd'" }
        when ($_ > 100)   { say "\$foo > 100" }
        default           { say "None of the above" }
    }

    use feature ':5.10'; # loads all features available in perl 5.10

    use v5.10;           # implicitly loads :5.10 feature bundle

=head1 DESCRIPTION

It is usually impossible to add new syntax to Perl without breaking
some existing programs.  This pragma provides a way to minimize that
risk. New syntactic constructs, or new semantic meanings to older
constructs, can be enabled by C<use feature 'foo'>, and will be parsed
only when the appropriate feature pragma is in scope.  (Nevertheless, the
C<CORE::> prefix provides access to all Perl keywords, regardless of this
pragma.)

=head2 Lexical effect

Like other pragmas (C<use strict>, for example), features have a lexical
effect.  C<use feature qw(foo)> will only make the feature "foo" available
from that point to the end of the enclosing block.

    {
        use feature 'say';
        say "say is available here";
    }
    print "But not here.\n";

=head2 C<no feature>

Features can also be turned off by using C<no feature "foo">.  This too
has lexical effect.

    use feature 'say';
    say "say is available here";
    {
        no feature 'say';
        print "But not here.\n";
    }
    say "Yet it is here.";

C<no feature> with no features specified will reset to the default group.  To
disable I<all> features (an unusual request!) use C<no feature ':all'>.

=head1 AVAILABLE FEATURES

=head2 The 'say' feature

C<use feature 'say'> tells the compiler to enable the Perl 6 style
C<say> function.

See L<perlfunc/say> for details.

This feature is available starting with Perl 5.10.

=head2 The 'state' feature

C<use feature 'state'> tells the compiler to enable C<state>
variables.

See L<perlsub/"Persistent Private Variables"> for details.

This feature is available starting with Perl 5.10.

=head2 The 'switch' feature

C<use feature 'switch'> tells the compiler to enable the Perl 6
given/when construct.

See L<perlsyn/"Switch Statements"> for details.

This feature is available starting with Perl 5.10.

=head2 The 'unicode_strings' feature

C<use feature 'unicode_strings'> tells the compiler to use Unicode semantics
in all string operations executed within its scope (unless they are also
within the scope of either C<use locale> or C<use bytes>).  The same applies
to all regular expressions compiled within the scope, even if executed outside
it.  It does not change the internal representation of strings, but only how
they are interpreted.

C<no feature 'unicode_strings'> tells the compiler to use the traditional
Perl semantics wherein the native character set semantics is used unless it is
clear to Perl that Unicode is desired.  This can lead to some surprises
when the behavior suddenly changes.  (See
L<perlunicode/The "Unicode Bug"> for details.)  For this reason, if you are
potentially using Unicode in your program, the
C<use feature 'unicode_strings'> subpragma is B<strongly> recommended.

This feature is available starting with Perl 5.12; was almost fully
implemented in Perl 5.14; and extended in Perl 5.16 to cover C<quotemeta>.

=head2 The 'unicode_eval' and 'evalbytes' features

Under the C<unicode_eval> feature, Perl's C<eval> function, when passed a
string, will evaluate it as a string of characters, ignoring any
C<use utf8> declarations.  C<use utf8> exists to declare the encoding of
the script, which only makes sense for a stream of bytes, not a string of
characters.  Source filters are forbidden, as they also really only make
sense on strings of bytes.  Any attempt to activate a source filter will
result in an error.

The C<evalbytes> feature enables the C<evalbytes> keyword, which evaluates
the argument passed to it as a string of bytes.  It dies if the string
contains any characters outside the 8-bit range.  Source filters work
within C<evalbytes>: they apply to the contents of the string being
evaluated.

Together, these two features are intended to replace the historical C<eval>
function, which has (at least) two bugs in it, that cannot easily be fixed
without breaking existing programs:

=over

=item *

C<eval> behaves differently depending on the internal encoding of the
string, sometimes treating its argument as a string of bytes, and sometimes
as a string of characters.

=item *

Source filters activated within C<eval> leak out into whichever I<file>
scope is currently being compiled.  To give an example with the CPAN module
L<Semi::Semicolons>:

    BEGIN { eval "use Semi::Semicolons;  # not filtered here " }
    # filtered here!

C<evalbytes> fixes that to work the way one would expect:

    use feature "evalbytes";
    BEGIN { evalbytes "use Semi::Semicolons;  # filtered " }
    # not filtered

=back

These two features are available starting with Perl 5.16.

=head2 The 'current_sub' feature

This provides the C<__SUB__> token that returns a reference to the current
subroutine or C<undef> outside of a subroutine.

This feature is available starting with Perl 5.16.

=head2 The 'array_base' feature

This feature supports the legacy C<$[> variable.  See L<perlvar/$[> and
L<arybase>.  It is on by default but disabled under C<use v5.16> (see
L</IMPLICIT LOADING>, below).

This feature is available under this name starting with Perl 5.16.  In
previous versions, it was simply on all the time, and this pragma knew
nothing about it.

=head2 The 'fc' feature

C<use feature 'fc'> tells the compiler to enable the C<fc> function,
which implements Unicode casefolding.

See L<perlfunc/fc> for details.

This feature is available from Perl 5.16 onwards.

=head2 The 'lexical_subs' feature

B<WARNING>: This feature is still experimental and the implementation may
change in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the
warning:

    no warnings "experimental::lexical_subs";

This enables declaration of subroutines via C<my sub foo>, C<state sub foo>
and C<our sub foo> syntax.  See L<perlsub/Lexical Subroutines> for details.

This feature is available from Perl 5.18 onwards.

=head2 The 'signatures' feature

B<WARNING>: This feature is still experimental and the implementation may
change in future versions of Perl.  For this reason, Perl will
warn when you use the feature, unless you have explicitly disabled the
warning:

    no warnings "experimental::signatures";

This enables unpacking of subroutine arguments into lexical variables
by syntax such as

    sub foo ($left, $right) {
	return $left + $right;
    }

See L<perlsub/Signatures> for details.

This feature is available from Perl 5.20 onwards.

=head1 FEATURE BUNDLES

It's possible to load multiple features together, using
a I<feature bundle>.  The name of a feature bundle is prefixed with
a colon, to distinguish it from an actual feature.

  use feature ":5.10";

The following feature bundles are available:

  bundle    features included
  --------- -----------------
  :default  array_base

  :5.10     say state switch array_base

  :5.12     say state switch unicode_strings array_base

  :5.14     say state switch unicode_strings array_base

  :5.16     say state switch unicode_strings
            unicode_eval evalbytes current_sub fc

  :5.18     say state switch unicode_strings
            unicode_eval evalbytes current_sub fc

  :5.20     say state switch unicode_strings
            unicode_eval evalbytes current_sub fc

The C<:default> bundle represents the feature set that is enabled before
any C<use feature> or C<no feature> declaration.

Specifying sub-versions such as the C<0> in C<5.14.0> in feature bundles has
no effect.  Feature bundles are guaranteed to be the same for all sub-versions.

  use feature ":5.14.0";    # same as ":5.14"
  use feature ":5.14.1";    # same as ":5.14"

=head1 IMPLICIT LOADING

Instead of loading feature bundles by name, it is easier to let Perl do
implicit loading of a feature bundle for you.

There are two ways to load the C<feature> pragma implicitly:

=over 4

=item *

By using the C<-E> switch on the Perl command-line instead of C<-e>.
That will enable the feature bundle for that version of Perl in the
main compilation unit (that is, the one-liner that follows C<-E>).

=item *

By explicitly requiring a minimum Perl version number for your program, with
the C<use VERSION> construct.  That is,

    use v5.10.0;

will do an implicit

    no feature ':all';
    use feature ':5.10';

and so on.  Note how the trailing sub-version
is automatically stripped from the
version.

But to avoid portability warnings (see L<perlfunc/use>), you may prefer:

    use 5.010;

with the same effect.

If the required version is older than Perl 5.10, the ":default" feature
bundle is automatically loaded instead.

=back

=cut

sub import {
    my $class = shift;

    if (!@_) {
        croak("No features specified");
    }

    __common(1, @_);
}

sub unimport {
    my $class = shift;

    # A bare C<no feature> should reset to the default bundle
    if (!@_) {
	$^H &= ~($hint_uni8bit|$hint_mask);
	return;
    }

    __common(0, @_);
}


sub __common {
    my $import = shift;
    my $bundle_number = $^H & $hint_mask;
    my $features = $bundle_number != $hint_mask
	&& $feature_bundle{$hint_bundles[$bundle_number >> $hint_shift]};
    if ($features) {
	# Features are enabled implicitly via bundle hints.
	# Delete any keys that may be left over from last time.
	delete @^H{ values(%feature) };
	$^H |= $hint_mask;
	for (@$features) {
	    $^H{$feature{$_}} = 1;
	    $^H |= $hint_uni8bit if $_ eq 'unicode_strings';
	}
    }
    while (@_) {
        my $name = shift;
        if (substr($name, 0, 1) eq ":") {
            my $v = substr($name, 1);
            if (!exists $feature_bundle{$v}) {
                $v =~ s/^([0-9]+)\.([0-9]+).[0-9]+$/$1.$2/;
                if (!exists $feature_bundle{$v}) {
                    unknown_feature_bundle(substr($name, 1));
                }
            }
            unshift @_, @{$feature_bundle{$v}};
            next;
        }
        if (!exists $feature{$name}) {
            unknown_feature($name);
        }
	if ($import) {
	    $^H{$feature{$name}} = 1;
	    $^H |= $hint_uni8bit if $name eq 'unicode_strings';
	} else {
            delete $^H{$feature{$name}};
            $^H &= ~ $hint_uni8bit if $name eq 'unicode_strings';
        }
    }
}

sub unknown_feature {
    my $feature = shift;
    croak(sprintf('Feature "%s" is not supported by Perl %vd',
            $feature, $^V));
}

sub unknown_feature_bundle {
    my $feature = shift;
    croak(sprintf('Feature bundle "%s" is not supported by Perl %vd',
            $feature, $^V));
}

sub croak {
    require Carp;
    Carp::croak(@_);
}

1;

# ex: set ro:
