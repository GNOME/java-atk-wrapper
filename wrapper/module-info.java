module atk.wrapper {
    exports org.GNOME.Accessibility;
    requires java.desktop;
    requires java.management;
    provides javax.accessibility.AccessibilityProvider
        with org.GNOME.Accessibility.AtkProvider;
}
