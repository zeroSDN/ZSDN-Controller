echo "### Building C++ Topics ###"

ZDIR=$(<zsdn-dir.txt)

SRC_DIR="${ZDIR}/common/module_interface_definitions"
OUT_DIR="${ZDIR}/common/cpp/zsdn-commons/zsdn/topics"

echo "Topics Source Dir: ${SRC_DIR}"
echo "Topics Out Dir: ${OUT_DIR}"

cd hierarchy-builder/hierarchy-builder/target/

java -jar ./hierarchy-builder-0.0.1-SNAPSHOT-jar-with-dependencies.jar -src ${SRC_DIR} -r -out ${OUT_DIR} -lang cpp

echo "### Finished Building C++ Topics ###"