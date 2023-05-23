#!/usr/bin/env groovy

def parallelStages = [:]
// def packagesToBuild = ["AdditionalMaps", "Town06_Opt", "Town07_Opt", "Town11", "Town12", "Town13"]
def packagesToBuild = ["AdditionalMaps"]

pipeline
{
    agent none

    options
    {
        buildDiscarder(logRotator(numToKeepStr: '3', artifactNumToKeepStr: '3'))
    }

    stages
    {
        stage('Creating nodes')
        {
            agent { label "master" }
            steps
            {
                script
                {
                    JOB_ID = "${env.BUILD_TAG}"
                    jenkinsLib = load("/home/jenkins/jenkins_426.groovy")

                    jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "AdditionalMaps")
                    // jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town06_Opt")
                    // jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town07_Opt")
                    // jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town11")
                    // jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town12")
                    // jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town13")
                    // jenkinsLib.CreateWindowsBuildNode(JOB_ID)
                }
            }
        }
        stage('Building CARLA')
        {
            parallel
            {
                stage('ubuntu')
                {
                    agent { label "ubuntu && build && UP_${JOB_ID}_AdditionalMaps" }
                    environment
                    {
                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                    }
                    stages
                    {
                        stage('ubuntu packages')
                        {
                            steps {
                                script {
                                    packagesToBuild.each { pkg ->
                                        parallelStages[pkg] = {
                                            // node("ubuntu && build && ${JOB_ID} && ${pkg}") {
                                            // node("UP_${JOB_ID}_${pkg}") {
                                            node() {
                                                stage(pkg) {
                                                    environment() {
                                                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                                                    }
                                                    sh('cd ~/carla && make package ARGS="--packages=${pkg} --python-version=3.7,2 --target-wheel-platform=manylinux_2_27_x86_64"')
                                                    post {
                                                        always {
                                                            archiveArtifacts 'Dist/*.tar.gz'
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    parallel(parallelStages)
                                }
                            }
                        }
                    }
                    post
                    {
                        always
                        {
                            deleteDir()

                            node('master')
                            {
                                script
                                {
                                    JOB_ID = "${env.BUILD_TAG}"
                                    jenkinsLib = load("/home/jenkins/jenkins_426.groovy")

                                    jenkinsLib.DeleteUbuntuBuildNode(JOB_ID)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
