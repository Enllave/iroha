#!/usr/bin/env groovy

def buildOptionsString(options) {
  def s = ''
  if (options) {
    options.each { k, v ->
      s += "--build-arg ${k}=${v} "
    }
  }
  return s
}

def dockerPullOrBuild(imageName, currentDockerfileURL, previousDockerfileURL, referenceDockerfileURL, scmVars, environment, buildOptions=null) {
  buildOptions = buildOptionsString(buildOptions)
  withEnv(environment) {
    def utils = load '.jenkinsci/utils/utils.groovy'
    randDir = sh(script: "cat /dev/urandom | tr -dc 'a-zA-Z0-9' | head -c 10", returnStdout: true).trim()
    currentDockerfile = utils.getUrl(currentDockerfileURL, "/tmp/${randDir}/currentDockerfile", true)
    previousDockerfile = utils.getUrl(previousDockerfileURL, "/tmp/${randDir}/previousDockerfile")
    referenceDockerfile = utils.getUrl(referenceDockerfileURL, "/tmp/${randDir}/referenceDockerfile")
    if (utils.filesDiffer(currentDockerfile, previousDockerfile) && utils.filesDiffer(currentDockerfile, referenceDockerfile)) {
      // Dockerfile has been changed compared to both the previous commit and reference Dockerfile
      // Worst case scenario. We cannot count on the local cache
      // because Dockerfile may contain apt-get entries that would try to update
      // from invalid (stale) addresses
      iC = docker.build("${env.DOCKER_REGISTRY_BASENAME}:${randDir}-${BUILD_NUMBER}", "${buildOptions} --no-cache -f ${currentDockerfile} .")
    }
    else {
      // first commit in this branch or Dockerfile modified
      if (utils.filesDiffer(currentDockerfile, referenceDockerfile)) {
        // if we're lucky to build on the same agent, image will be built using cache
        iC = docker.build("${env.DOCKER_REGISTRY_BASENAME}:${randDir}-${BUILD_NUMBER}", "$buildOptions -f ${currentDockerfile} .")
      }
      else {
        // try pulling image from Dockerhub, probably image is already there
        def testExitCode = sh(script: "docker pull ${env.DOCKER_REGISTRY_BASENAME}:${imageName}", returnStatus: true)
        if (testExitCode != 0) {
          // image does not (yet) exist on Dockerhub. Build it
          iC = docker.build("${env.DOCKER_REGISTRY_BASENAME}:${randDir}-${BUILD_NUMBER}", "$buildOptions --no-cache -f ${currentDockerfile} .")
        }
        else {
          // no difference found compared to both previous and reference Dockerfile
          iC = docker.image("${env.DOCKER_REGISTRY_BASENAME}:${imageName}")
        }
      }
    }
  }
  return iC
  // if (GIT_LOCAL_BRANCH ==~ /develop|master|dev/ || CHANGE_BRANCH_LOCAL == 'develop' || CHANGE_BRANCH_LOCAL == 'dev') {
  //   docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
  //     iC.push(imageName)
  //   }
  // }
  // return iC
}

return this