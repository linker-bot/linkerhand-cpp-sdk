<#
.SYNOPSIS
    LinkerHand C++ SDK 一条命令安装脚本 (Windows x64)

.DESCRIPTION
    从 GitHub Release 下载对应版本的 zip，校验 SHA256，解压到 $env:ProgramFiles\LinkerHand\cpp-sdk，
    并把安装前缀加入用户 CMAKE_PREFIX_PATH 环境变量。

.EXAMPLE
    iwr https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.ps1 -UseBasicParsing | iex

.EXAMPLE
    # 指定版本和路径
    & ([scriptblock]::Create((iwr https://raw.githubusercontent.com/linker-bot/linkerhand-cpp-sdk/main/scripts/install.ps1 -UseBasicParsing).Content)) -Version v2.1.8 -Prefix "C:\SDKs\LinkerHand"
#>

[CmdletBinding()]
param(
    [string]$Version = "",
    [string]$Prefix  = "$env:ProgramFiles\LinkerHand\cpp-sdk",
    [switch]$AddToPath
)

$ErrorActionPreference = 'Stop'
$Repo = 'linker-bot/linkerhand-cpp-sdk'

function Info($msg){ Write-Host "[INFO] $msg" -ForegroundColor Green }
function Warn($msg){ Write-Host "[WARN] $msg" -ForegroundColor Yellow }

# 解析版本 → tag
if ([string]::IsNullOrWhiteSpace($Version)) {
    Info "查询最新 Release..."
    $latest = Invoke-RestMethod "https://api.github.com/repos/$Repo/releases/latest"
    $Tag = $latest.tag_name
} else {
    $Tag = if ($Version.StartsWith('v')) { $Version } else { "v$Version" }
}
$Ver = $Tag.TrimStart('v')
Info "目标版本: $Tag"

$Pkg   = "linkerhand-cpp-sdk-$Ver-windows-x64"
$Zip   = "$Pkg.zip"
$Base  = "https://github.com/$Repo/releases/download/$Tag"

$Tmp = Join-Path $env:TEMP ("linkerhand-sdk-install-" + [Guid]::NewGuid().ToString('N').Substring(0,8))
New-Item -ItemType Directory -Path $Tmp -Force | Out-Null
try {
    $zipPath  = Join-Path $Tmp $Zip
    $sumsPath = Join-Path $Tmp 'SHA256SUMS'

    Info "下载: $Base/$Zip"
    Invoke-WebRequest -Uri "$Base/$Zip"           -OutFile $zipPath  -UseBasicParsing
    Info "下载: $Base/SHA256SUMS"
    Invoke-WebRequest -Uri "$Base/SHA256SUMS"     -OutFile $sumsPath -UseBasicParsing

    # 校验 SHA256（SHA256SUMS 格式："<hash>  <filename>"）
    Info "校验 SHA256..."
    $expected = (Get-Content $sumsPath | Where-Object { $_ -match "\s+$([regex]::Escape($Zip))\s*$" } | Select-Object -First 1) -split '\s+' | Select-Object -First 1
    if (-not $expected) { throw "SHA256SUMS 中未找到 $Zip 的记录" }
    $actual = (Get-FileHash $zipPath -Algorithm SHA256).Hash.ToLower()
    if ($actual -ne $expected.ToLower()) {
        throw "SHA256 校验失败：expected=$expected actual=$actual"
    }
    Info "校验通过"

    # 解压 → 拷贝到 Prefix
    Info "解压..."
    Expand-Archive -Path $zipPath -DestinationPath $Tmp -Force
    $stage = Join-Path $Tmp $Pkg
    if (-not (Test-Path $stage)) { throw "解压后未找到目录：$stage" }

    Info "安装到 $Prefix ..."
    if (Test-Path $Prefix) {
        # 只清理 SDK 自身条目，不误伤同前缀下用户其他内容
        Remove-Item -Recurse -Force (Join-Path $Prefix 'include\linkerhand-cpp-sdk') -ErrorAction SilentlyContinue
        Remove-Item -Recurse -Force (Join-Path $Prefix 'lib\linkerhand-cpp-sdk')     -ErrorAction SilentlyContinue
        Remove-Item -Recurse -Force (Join-Path $Prefix 'lib\cmake\linkerhand-cpp-sdk') -ErrorAction SilentlyContinue
    }
    New-Item -ItemType Directory -Path $Prefix -Force | Out-Null
    Copy-Item -Path (Join-Path $stage '*') -Destination $Prefix -Recurse -Force

    # 更新用户环境变量 CMAKE_PREFIX_PATH（追加而非覆盖）
    $curCPP = [Environment]::GetEnvironmentVariable('CMAKE_PREFIX_PATH', 'User')
    if ([string]::IsNullOrEmpty($curCPP)) {
        [Environment]::SetEnvironmentVariable('CMAKE_PREFIX_PATH', $Prefix, 'User')
    } elseif (($curCPP -split ';') -notcontains $Prefix) {
        [Environment]::SetEnvironmentVariable('CMAKE_PREFIX_PATH', "$curCPP;$Prefix", 'User')
    }

    if ($AddToPath) {
        $binDir = Join-Path $Prefix 'bin'
        if (Test-Path $binDir) {
            $curPath = [Environment]::GetEnvironmentVariable('Path', 'User')
            if (($curPath -split ';') -notcontains $binDir) {
                [Environment]::SetEnvironmentVariable('Path', "$curPath;$binDir", 'User')
                Info "已将 $binDir 加入用户 PATH（新终端生效）"
            }
        }
    }

    Info "安装完成 ✓"
    Write-Host ""
    Write-Host "版本   : $Tag"
    Write-Host "前缀   : $Prefix"
    Write-Host "CMake  : find_package(linkerhand-cpp-sdk CONFIG REQUIRED)"
    Write-Host "        （已写入用户 CMAKE_PREFIX_PATH，新终端生效）"
} finally {
    Remove-Item -Recurse -Force $Tmp -ErrorAction SilentlyContinue
}
