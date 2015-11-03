echo "### Building JavaTopics ###"

ZDIR=$(<zsdn-dir.txt)

SRC_DIR="${ZDIR}/common/module_interface_definitions"
OUT_DIR="${ZDIR}/common/java/zsdn-proto/src/main/java/zsdn/topics/"

echo "Topics Source Dir: ${SRC_DIR}"
echo "Topics Out Dir: ${OUT_DIR}"

cd hierarchy-builder/hierarchy-builder/target/

java -jar ./hierarchy-builder-0.0.1-SNAPSHOT-jar-with-dependencies.jar -src ${SRC_DIR} -r -out ${OUT_DIR} -lang java

echo "### Finished Building Java Topics ###"