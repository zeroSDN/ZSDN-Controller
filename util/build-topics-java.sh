echo "### Building JavaTopics ###"

ZDIR=$(<zsdn-dir.txt)

SRC_DIR="${ZDIR}/modules"
OUT_DIR="${ZDIR}/common/java/zsdn-proto/src/main/java/zsdn/topics/"

echo "Topics Source Dir: ${SRC_DIR}"
echo "Topics Out Dir: ${OUT_DIR}"

cd hierarchy-builder/hierarchy-builder/target/

java -jar ./hierarchy-builder-0.0.1-SNAPSHOT-jar-with-dependencies.jar --source ${SRC_DIR} -r --out ${OUT_DIR} --language java --java_package "zsdn.topics"

echo "### Finished Building Java Topics ###"
