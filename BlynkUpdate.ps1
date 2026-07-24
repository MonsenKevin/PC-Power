# Enable both TLS 1.2 and TLS 1.3
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 -bor [Net.SecurityProtocolType]::Tls13

# Create headers that mimic Google Chrome
$headers = @{
    "User-Agent" = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
}

try {
    # Attempt to visit the site using the fake Chrome headers
    Invoke-RestMethod -Uri "https://ny3.blynk.cloud/external/api/update?token=YOUR_BLYNK_TOKEN_HERE&v3=1" -Headers $headers -TimeoutSec 10
    Write-Output "Success! The website allowed the connection."
} catch {
    Write-Output "Error: $_"
}