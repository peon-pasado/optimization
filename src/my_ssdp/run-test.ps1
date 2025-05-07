$exe = ".\main.exe"

if (-not (Test-Path $exe)) {
    Write-Host "No se encontró main.exe"
    exit 1
}

$total = 0
$min_time = [int]::MaxValue
$max_time = 0
$ncasos = 0
$tiempos = @()

Get-ChildItem -Directory -Filter "wt*" | ForEach-Object {
    $folder = $_.Name
    $xxx = $folder.Substring(2)
    $optfile = "opt$xxx.in"

    if (-not (Test-Path $optfile)) {
        Write-Host "Archivo $optfile no encontrado, se omite carpeta $folder"
        return
    }

    Write-Host "Procesando $folder con respuestas en $optfile"

    for ($i = 1; $i -le 125; $i++) {
        $yyy = $i.ToString("D3")
        $datfile = "$folder\$folder" + "_$yyy.dat"

        if (Test-Path $datfile) {
            $start = Get-Date
            $salida = Get-Content $datfile | & $exe
            $end = Get-Date

            $dur_ms = [math]::Round(($end - $start).TotalMilliseconds)
            $tiempos += $dur_ms
            $total += $dur_ms
            $ncasos += 1
            if ($dur_ms -lt $min_time) { $min_time = $dur_ms }
            if ($dur_ms -gt $max_time) { $max_time = $dur_ms }

            $esperada = Get-Content $optfile | Select-Object -Index ($i - 1)

            if ($salida -eq $esperada) {
                Write-Host "[$folder/$yyy] OK ($dur_ms ms)"
            } else {
                Write-Host "[$folder/$yyy] ERROR ($dur_ms ms)"
                Write-Host "  Esperado: $esperada"
                Write-Host "  Obtenido: $salida"
            }
        }
    }
}

if ($ncasos -gt 0) {
    $prom = [math]::Round($total / $ncasos)
    Write-Host ""
    Write-Host "Estadísticas de tiempo (ms):"
    Write-Host "  Casos ejecutados: $ncasos"
    Write-Host "  Total: $total"
    Write-Host "  Promedio: $prom"
    Write-Host "  Mínimo: $min_time"
    Write-Host "  Máximo: $max_time"
} else {
    Write-Host "No se ejecuto ningun caso."
}
