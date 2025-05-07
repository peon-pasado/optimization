param (
    [Parameter(Mandatory = $true)]
    [string]$XXX
)

$FOLDER = "wt$XXX"
$EXEC = ".\sips.exe"

if (-not (Test-Path $EXEC -PathType Leaf)) {
    Write-Error "No se encontró el ejecutable: $EXEC"
    exit 2
}

if (-not (Test-Path $FOLDER -PathType Container)) {
    Write-Error "No se encontró la carpeta: $FOLDER"
    exit 3
}

# No imprimir "Procesando $FOLDER..."
$tiempos = @()
$total = 0
$min_time = [int]::MaxValue
$max_time = 0
$ncasos = 0

foreach ($i in 1..125) {
    $yyy = $i.ToString("D3")
    $datfile = "$FOLDER\${FOLDER}_$yyy.dat"

    if (Test-Path $datfile -PathType Leaf) {
        $start = Get-Date

        # Ejecutar el programa y redirigir la salida estándar y de error a $null utilizando Out-Null
        & $EXEC -v 0 $datfile | Out-Null 2>&1

        $end = Get-Date
        $duration = ($end - $start).TotalMilliseconds
        $dur_ms = [math]::Round($duration)

        # Acumular los tiempos
        $tiempos += $dur_ms
        $total += $dur_ms
        $ncasos += 1

        # Calcular los tiempos mínimo y máximo
        if ($dur_ms -lt $min_time) { $min_time = $dur_ms }
        if ($dur_ms -gt $max_time) { $max_time = $dur_ms }

        # Imprimir solo los tiempos del script (sin salida de sips.exe)
        Write-Output "[$FOLDER/$yyy] Ejecutado en ${dur_ms} ms"
    }
}

# Solo imprimir estadísticas finales, sin los detalles de cada archivo
if ($ncasos -gt 0) {
    $prom = [math]::Round($total / $ncasos)
    
    # Imprimir estadísticas finales
    Write-Host ""
    Write-Host "Estadísticas de tiempo (ms):"
    Write-Host "  Casos ejecutados: $ncasos"
    Write-Host "  Total: $total"
    Write-Host "  Promedio: $prom"
    Write-Host "  Mínimo: $min_time"
    Write-Host "  Máximo: $max_time"
} else {
    # Imprimir un mensaje si no se ejecutaron casos
    Write-Host "No se ejecutó ningún caso."
}
