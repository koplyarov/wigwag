function OutputParser
{
	Begin
	{
		$r = New-Object 'System.Collections.Generic.SortedDictionary[string,object]'
	}
	Process
	{
		$m = [regex]::Match($_, "^<(.*) finished: (.*)>$")
		if ($m.Success)
		{
			$op = $m.Groups[1].Value
			$ns = $m.Groups[2].Value -as [int]
			$r.Add("${op}, ns", $ns)
		}
		$m = [regex]::Match($_, "^<measure memory, name: (.*), count: (.*)>$")
		if ($m.Success)
		{
			$name = $m.Groups[1].Value
			$count = $m.Groups[2].Value -as [int]
			$rss = (ps wigwag_benchmarks).WorkingSet64
			$r.Add("memory per ${name}", $($rss / $count -as [int]))
		}
	}
	End
	{
		return $r
	}
}


function Benchmark($task, $obj, $count, $secondary_count)
{
	$a = @("--task", $task, "--obj", $obj, "--count", $count)
	if ($secondary_count -ne $null)
	{
		$a += @("--secondary-count", $secondary_count)
	}

	$data = &"$benchmarks_dir\wigwag_benchmarks" @a | OutputParser
	$r = New-Object 'System.Collections.Generic.SortedDictionary[string,object]'
	$r.Add('name', "$task $obj $count $secondary_count")
	$r.Add("data", $data)
	sleep -s 1
	return $r
}


function GetSystemInfo
{
	$r = New-Object 'System.Collections.Generic.SortedDictionary[string,object]'
	$r.Add('os', [System.Environment]::OSVersion.VersionString)
	$r.Add('cpu', $(Get-WmiObject Win32_Processor).Name)
	return $r
}


for ($i = 0; $i -lt $args.Length; ++$i)
{
	switch ($args[$i])
	{
		--bin-dir { $benchmarks_dir = "$($args[++$i])"  }
		--file { $filename = "$($args[++$i])" }
		default { [Console]::Error.WriteLine("Unknown option: $($args[$i])"); exit 1 }
	}
}

if ($filename -eq $null) { $filename = "benchmarks\benchmarks.list" }
if ($benchmarks_dir -eq $null) { $benchmarks_dir = "$PSScriptRoot\..\build\bin\Release" }

$result = New-Object 'System.Collections.Generic.SortedDictionary[string,object]'
$result.Add('script', $filename)
$result.Add('systemInfo', (GetSystemInfo))
$result.Add('results', (New-Object 'System.Collections.Generic.SortedDictionary[string,object]'))

foreach ($line in $(Get-Content $filename))
{
	if ($line -match "^#" ) { continue }
	if ($line -match "^\s*$") { continue }

	$benchmark_args = $line -Split "\s+"
	$b = Benchmark @benchmark_args
	$result.results.Add($b.name, $b.data)
}

ConvertTo-Json $result
