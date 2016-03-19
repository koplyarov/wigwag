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

$num_passes = 30

mkdir -Force "$out_dir"
foreach ($f in dir "$PSScriptRoot\..\benchmarks")
{
	$results = $null
	for ($i = 0; $i -lt $num_passes; ++$i)
	{
		echo "$($f.BaseName): $i"
		$tmp = &"$PSScriptRoot\run_benchmarks.ps1" --file "$($f.FullName)" | ConvertFrom-Json
		if ($results -eq $null)
		{
			$results = $tmp
		}
		else
		{
			foreach ($b in $tmp.results.PSObject.Properties)
			{
				$out_results = $results.results.PSObject.Properties[$b.Name].Value
				$tmp_results = $b.Value
				foreach ($k in $tmp_results.PSObject.Properties)
				{
					$a = $out_results.PSObject.Properties[$k.Name].Value
					$b = $tmp_results.PSObject.Properties[$k.Name].Value
					$out_results.PSObject.Properties[$k.Name].Value = [Math]::Min($a, $b)
				}
			}
		}
	}
	$results | ConvertTo-Json > "$out_dir/$($f.BaseName).json"
}
