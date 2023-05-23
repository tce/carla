#!/usr/bin/env groovy

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
                    jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town06_Opt")
                    jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town07_Opt")
                    jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town11")
                    jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town12")
                    jenkinsLib.CreateUbuntuBuildNodePackage(JOB_ID, "Town13")
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
                    agent { label "ubuntu && build && ${JOB_ID}" }
                    environment
                    {
                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                    }
                    stages
                    {
                        stage('ubuntu packages')
                        {
                            parallel
                            {
                                stage('ubuntu package AdditionalMaps')
                                {
                                    agent { label "ubuntu && build && ${JOB_ID} && AdditionalMaps" }
                                    environment
                                    {
                                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                                    }
                                    steps
                                    {
                                        sh '''
                                            cd ~/carla
                                            make package ARGS="--packages=AdditionalMaps --python-version=3.7,2 --target-wheel-platform=manylinux_2_27_x86_64"
                                        '''
                                    }
                                    post
                                    {
                                        always
                                        {
                                            archiveArtifacts 'Dist/*.tar.gz'
                                            // stash includes: 'Dist/CARLA*.tar.gz', name: 'ubuntu_package'
                                        }
                                    }
                                }
                                stage('ubuntu package Town06_Opt')
                                {
                                    agent { label "ubuntu && build && ${JOB_ID} && Town06_Opt" }
                                    environment
                                    {
                                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                                    }
                                    steps
                                    {
                                        sh '''
                                            cd ~/carla
                                            make package ARGS="--packages=Town06_Opt --clean-intermediate --python-version=3.7,2 --target-wheel-platform=manylinux_2_27_x86_64"
                                        '''
                                    }
                                    post
                                    {
                                        always
                                        {
                                            archiveArtifacts 'Dist/*.tar.gz'
                                            // stash includes: 'Dist/CARLA*.tar.gz', name: 'ubuntu_package'
                                        }
                                    }
                                }
                                stage('ubuntu package Town07_Opt')
                                {
                                    agent { label "ubuntu && build && ${JOB_ID} && Town07_Opt" }
                                    environment
                                    {
                                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                                    }
                                    steps
                                    {
                                        sh '''
                                            cd ~/carla
                                            make package ARGS="--packages=Town07_Opt --clean-intermediate --python-version=3.7,2 --target-wheel-platform=manylinux_2_27_x86_64"
                                        '''
                                    }
                                    post
                                    {
                                        always
                                        {
                                            archiveArtifacts 'Dist/*.tar.gz'
                                            // stash includes: 'Dist/CARLA*.tar.gz', name: 'ubuntu_package'
                                        }
                                    }
                                }
                                stage('ubuntu package Town11')
                                {
                                    agent { label "ubuntu && build && ${JOB_ID} && Town11" }
                                    environment
                                    {
                                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                                    }
                                    steps
                                    {
                                        sh '''
                                            cd ~/carla
                                            make package ARGS="--packages=Town11 --clean-intermediate --python-version=3.7,2 --target-wheel-platform=manylinux_2_27_x86_64"
                                        '''
                                    }
                                    post
                                    {
                                        always
                                        {
                                            archiveArtifacts 'Dist/*.tar.gz'
                                            // stash includes: 'Dist/CARLA*.tar.gz', name: 'ubuntu_package'
                                        }
                                    }
                                }
                                stage('ubuntu package Town12')
                                {
                                    agent { label "ubuntu && build && ${JOB_ID} && Town12" }
                                    environment
                                    {
                                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                                    }
                                    steps
                                    {
                                        sh '''
                                            cd ~/carla
                                            make package ARGS="--packages=Town12 --clean-intermediate --python-version=3.7,2 --target-wheel-platform=manylinux_2_27_x86_64"
                                        '''
                                    }
                                    post
                                    {
                                        always
                                        {
                                            archiveArtifacts 'Dist/*.tar.gz'
                                            // stash includes: 'Dist/CARLA*.tar.gz', name: 'ubuntu_package'
                                        }
                                    }
                                }
                                stage('ubuntu package Town13')
                                {
                                    agent { label "ubuntu && build && ${JOB_ID} && Town13" }
                                    environment
                                    {
                                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.26'
                                    }
                                    steps
                                    {
                                        sh '''
                                            cd ~/carla
                                            make package ARGS="--packages=Town13 --clean-intermediate --python-version=3.7,2 --target-wheel-platform=manylinux_2_27_x86_64"
                                        '''
                                    }
                                    post
                                    {
                                        always
                                        {
                                            archiveArtifacts 'Dist/*.tar.gz'
                                            // stash includes: 'Dist/CARLA*.tar.gz', name: 'ubuntu_package'
                                        }
                                    }
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
                stage('windows')
                {
                    agent { label "windows && build && ${JOB_ID}" }
                    environment
                    {
                        UE4_ROOT = 'C:\\UE_4.26'
                    }
                    stages
                    {
                        stage('windows setup')
                        {
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    git update-index --skip-worktree Unreal/CarlaUE4/CarlaUE4.uproject
                                """
                                bat """
                                    call ../setEnv64.bat
                                    make setup ARGS="--chrono"
                                """
                            }
                        }
                        stage('windows build')
                        {
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    make LibCarla
                                """
                                bat """
                                    call ../setEnv64.bat
                                    make PythonAPI
                                """
                                bat """
                                    call ../setEnv64.bat
                                    make CarlaUE4Editor ARGS="--chrono"
                                """
                                bat """
                                    call ../setEnv64.bat
                                    make plugins
                                """
                            }
                            post
                            {
                                always
                                {
                                    archiveArtifacts 'PythonAPI/carla/dist/*.egg'
                                    archiveArtifacts 'PythonAPI/carla/dist/*.whl'
                                }
                            }
                        }
                        stage('windows retrieve content')
                        {
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    call Update.bat
                                """
                            }
                        }
                        stage('windows package')
                        {
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    make package ARGS="--chrono"
                                """
                                bat """
                                    call ../setEnv64.bat
                                    make package ARGS="--packages=AdditionalMaps,Town06_Opt,Town07_Opt,Town11,Town12 --target-archive=AdditionalMaps --clean-intermediate"
                                """
                            }
                            post {
                                always {
                                    archiveArtifacts 'Build/UE4Carla/*.zip'
                                }
                            }
                        }
                        stage('windows deploy')
                        {
                            when { anyOf { branch "master"; branch "dev"; buildingTag() } }
                            steps {
                                bat """
                                    call ../setEnv64.bat
                                    git checkout .
                                    make deploy ARGS="--replace-latest"
                                """
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

                                    jenkinsLib.DeleteWindowsBuildNode(JOB_ID)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
