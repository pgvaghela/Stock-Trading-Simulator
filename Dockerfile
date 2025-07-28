FROM openjdk:17-jdk-slim

# Tell EB's nginx proxy to forward 80→8080
EXPOSE 8080

VOLUME /tmp
COPY demo/target/demo-0.0.1-SNAPSHOT.jar app.jar
ENTRYPOINT ["java","-jar","/app.jar"]