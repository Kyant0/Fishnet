plugins {
    alias(libs.plugins.android.library)
    id("com.vanniktech.maven.publish")
}

android {
    namespace = "com.kyant.fishnet"
    compileSdk {
        version = release(36)
    }
    buildToolsVersion = "36.1.0"
    ndkVersion = "29.0.14206865"

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
            version = "4.1.2"
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

mavenPublishing {
    publishToMavenCentral()
    signAllPublications()

    coordinates("io.github.kyant0", "fishnet", "1.0.5")

    pom {
        name.set("Fishnet")
        description.set("Dump Java, native and ANR crashes")
        inceptionYear.set("2025")
        url.set("https://github.com/Kyant0/Fishnet")
        licenses {
            license {
                name.set("The Apache License, Version 2.0")
                url.set("http://www.apache.org/licenses/LICENSE-2.0.txt")
                distribution.set("repo")
            }
        }
        developers {
            developer {
                id.set("Kyant0")
                name.set("Kyant")
                url.set("https://github.com/Kyant0")
            }
        }
        scm {
            url.set("https://github.com/Kyant0/Fishnet")
            connection.set("scm:git:git://github.com/Kyant0/Fishnet.git")
            developerConnection.set("scm:git:ssh://git@github.com/Kyant0/Fishnet.git")
        }
    }
}
