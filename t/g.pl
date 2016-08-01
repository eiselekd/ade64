$i = 0;
@i = ();
while (<>) {
    next if ($_ =~ /\(bad\)/);
    next if ($_ =~ /\.byte/);
    next if ($_ =~ /data16 nopw/);
    next if ($_ =~ /data16 data16/);
    next if ($_ =~ /rex\.[RWXB]+$/);
    next if ($_ =~ /rex$/);
    next if ($_ =~ /\s+data16$/);
    next if ($_ =~ /\s+ds$/);
    next if ($_ =~ /\s+cs$/);
    next if ($_ =~ /\s+gs$/);
    next if ($_ =~ /\s+ss$/);
    next if ($_ =~ /\s+es$/);
    next if ($_ =~ /\s+fs$/);
    next if ($_ =~ /<_end\+/);

    if ($_ =~ /^\s*[0-9a-f]+:\t((?:[0-9a-f]{2}[ ])+)$/) {
	my ($v,$line) = ($1,$_);
	my $off = scalar(@i);
	$i[$off-1][0] = $i[$off-1][0].$v;
	$i[$off-1][1] = $i[$off-1][1].$v;
    } elsif ($_ =~ /^\s*[0-9a-f]+:\t((?:[0-9a-f]{2}[ ])+)/) {
	my ($v,$line,$linepost) = ($1,$&,$');
	$line =~ s/\n//gms;
	push(@i,[$v,$line,$linepost]);
    }
}

my $i = 0;
foreach my $v (@i) {
    my $v0 = $$v[0];
    my $line = $$v[1].$$v[2];
    my $v = $v0;

    $v0 =~ s/\s*//g;
    $l = length($v0) / 2;

    next if ($v =~ /^c4/); # dont support vex
    next if ($v =~ /^c5/); # dont support vex

    print("v=`./p0.exe \'$v\'`; if [ \"\$v\" != \"$l\" ]; then\n");
    print("  echo \"Mismatch $i:(\$v!= expect:$l) $line\"\n");
    print("else\n");
    print("  echo \"OK       $i:$line\"\n");
    print("fi\n");
    $i++;
}
