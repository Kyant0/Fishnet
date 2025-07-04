plugins {
    alias(libs.plugins.android.library)
}

android {
    namespace = "com.kyant.fishnet"
    compileSdk = 36
    buildToolsVersion = "36.0.0"
    ndkVersion = "29.0.13599879"

    defaultConfig {
        minSdk = 21

        ndk {
            abiFilters += arrayOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
        }
        consumerProguardFiles("consumer-rules.pro")
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"), "proguard-rules.pro")
        }
    }
    externalNativeBuild {
        cmake {
            path("src/main/cpp/CMakeLists.txt")
            version = "4.0.2"
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_21
        targetCompatibility = JavaVersion.VERSION_21
    }
    lint {
        checkReleaseBuilds = false
    }
}
