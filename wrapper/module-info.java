module atk.wrapper {
    exports org.GNOME.Accessibility;
    requires transitive java.desktop;
    requires java.management;
    provides javax.accessibility.AccessibilityProvider
        with org.GNOME.Accessibility.AtkProvider;
}
