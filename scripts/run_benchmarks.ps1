$tabstop = 48

function OutputParser
{
	Process
	{
		$m = [regex]::Match($_, "^<(.*) finished: (.*)>$")
		if ($m.Success)
		{
			$op = $m.Groups[1].Value
			$ns = $m.Groups[2].Value
			"  {0,-$tabstop}{1,-8}" -f @("${op}:", "$ns ns")
		}
		$m = [regex]::Match($_, "^<measure memory, name: (.*), count: (.*)>$")
		if ($m.Success)
		{
			$name = $m.Groups[1].Value
			$count = $m.Groups[2].Value -as [int]
			$rss = (ps wigwag_benchmarks).WorkingSet64
			"  {0,-$tabstop}{1,-8}" -f @("memory per ${name}:", "$($rss / $count -as [int]) bytes")
		}
	}
}


function Benchmark($task, $obj, $count, $secondary_count)
{
	$a = @("--task", $task, "--obj", $obj, "--count", $count)
	if ($secondary_count -ne $null)
	{
		$a += @("--secondary-count", $secondary_count)
	}

	echo "=== $task $obj $count $secondary_count ==="
	&"$benchmarks_dir\wigwag_benchmarks" @a | OutputParser
	sleep -s 1
}


function PrintSystemInfo
{
	"### System info ###"
	"  $([System.Environment]::OSVersion.VersionString)"
	"  $((Get-WmiObject Win32_Processor).Name)"
}


for ($i = 0; $i -lt $args.Length; ++$i)
{
	switch ($args[$i])
	{
		--bin-dir { $benchmarks_dir = "$($args[++$i])"  }
		--file { $filename = "$($args[++$i])" }
		default { echo "Unknown option: $($args[$i])"; exit 1 }
	}
}

if ($filename -eq $null) { $filename = "benchmarks\benchmarks.list" }
if ($benchmarks_dir -eq $null) { $benchmarks_dir = "$PSScriptRoot\..\build\bin\Release" }

echo "Benchmark script: $filename"
echo ""

PrintSystemInfo
echo ""

$skip_empty_lines = $True
foreach ($line in $(Get-Content $filename))
{
	if ($line -match "^#" ) { continue }
	if ($line -match "^\s*$") 
	{ 
		if (-not $skip_empty_lines) { echo "" }
		continue
	}
	$benchmark_args = $line -Split "\s+"
	Benchmark @benchmark_args
	$skip_empty_lines = $False
}
