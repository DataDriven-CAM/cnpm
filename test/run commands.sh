rm -rf ./*/

export CNPM_HOME=`pwd`/..
export PATH=${CNPM_HOME}:${PATH}

cnpm prefix -g

mkdir -p myproject
cd myproject
rm -rf package.json

printf "\n\nbeautiful\nme\n\n\n\n" | cnpm init
printf "\n"
cnpm prefix
cnpm add DataDriven-CAM/json-thresher
cnpm --help