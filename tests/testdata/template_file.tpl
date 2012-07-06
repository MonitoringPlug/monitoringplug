[%# Test File for mp_template -%]
f[% 'o' -%][% IF 1 < 2 %]o[% END %][% IF 1 > 2 %]o[% END # COMMENT
-%]
b[% SWITCH 42 %][% CASE 23 %]A[% CASE 42 %]a[% END %]r[%# -%]
