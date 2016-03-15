for ($i = 0; $i -lt $args.Length; ++$i)
{
	switch ($args[$i])
	{
		--bin-dir { $benchmarks_dir = "$($args[++$i])"  }
		--out-dir { $out_dir = "$($args[++$i])" }
		default { echo "Unknown option: $($args[$i])"; exit 1 }
	}
}

if ($benchmarks_dir -eq $null) { $benchmarks_dir = "$PSScriptRoot\..\build\bin\Release" }
if ($out_dir -eq $null) { $out_dir = "$PSScriptRoot\..\benchmarks.out" }

mkdir -Force "$out_dir"
foreach ($f in dir "$PSScriptRoot\..\benchmarks")
{
	for ($i = 0; $i -lt 10; ++$i)
	{
		echo "$($f.BaseName): $i"
		&"$PSScriptRoot\run_benchmarks.ps1" --file "$($f.FullName)" > "$out_dir/$($f.BaseName).$i.out"
	}
}
