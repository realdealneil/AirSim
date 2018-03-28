#!/bin/bash

set -x
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "$SCRIPT_DIR" >/dev/null

UnrealDir=$1
if [[ ! -e "$UnrealDir" ]]; then
    # UnrealDir variable must be set like '/Users/Shared/Epic\ Games/UE_4.16'
    echo "UnrealDir is not set."
    exit 1
fi

if [ "$(uname)" == "Darwin" ]; then
    OS=Mac
else
    OS=Linux # Assume Linux
fi

# Call UnrealEngine shell scrpit
pushd "$UnrealDir/Engine/Build/BatchFiles/$OS/" >/dev/null
./GenerateProjectFiles.sh "$SCRIPT_DIR/Blocks.uproject"
popd >/dev/null

popd >/dev/null
set +x
