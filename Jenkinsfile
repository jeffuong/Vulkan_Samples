pipeline {
  agent none
  stages {
    stage('Build') {
      agent {
        label 'linux'
      }
      steps {
        checkout scm
        dir("src/hello_triangle") {
          sh 'make config=RELEASE'
        } 
      }//steps
    } //stage Build
    stage('Test') {
      agent {
        label 'linux'
      }
      steps {
        checkout scm
        dir("src/hello_triangle") {
          sh 'make test'
          sh 'make clean'
        }
      }
    }
  } //stages

}// pipeline