import com.android.build.gradle.internal.tasks.AppMetadataTask
import org.jetbrains.kotlin.gradle.dsl.JvmTarget
import org.jetbrains.kotlin.gradle.dsl.KotlinVersion

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
}

android {
    namespace = "com.kyant.fishnet.demo"
    compileSdk = 36
    buildToolsVersion = "36.1.0"
    ndkVersion = "29.0.14206865"

    defaultConfig {
        applicationId = namespace
        minSdk = 21
        targetSdk = 36
        versionCode = 4
        versionName = "1.0.3"

        ndk {
            abiFilters += arrayOf("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
        }
    }
    buildTypes {
        release {
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
            signingConfigs {
                create("release") {
                    enableV1Signing = true
                    enableV2Signing = true
                    enableV3Signing = true
                    enableV4Signing = false
                }
            }
            signingConfig = signingConfigs.getByName("release")
            vcsInfo {
                include = false
            }
        }
    }
    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
            version = "4.1.2"
        }
    }
    kotlin {
        jvmToolchain(21)
    }
    packaging {
        resources {
            excludes += arrayOf(
                "DebugProbesKt.bin",
                "kotlin/**",
                "META-INF/*.version",
                "META-INF/**/LICENSE.txt",
                "*.properties",
                "kotlin-tooling-metadata.json",
            )
        }
        dex {
            useLegacyPackaging = true
        }
        jniLibs {
            useLegacyPackaging = true
        }
    }
    dependenciesInfo {
        includeInApk = false
        includeInBundle = false
    }
    lint {
        checkReleaseBuilds = false
    }
}

tasks.withType<AppMetadataTask> {
    doLast {
        outputFile.asFile.get().writeBytes(byteArrayOf())
    }
}

dependencies {
    implementation(project(":fishnet"))
}
